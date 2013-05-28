#include "xmlsecrb.h"

static int addRubyKeyToManager(VALUE rb_key, VALUE rb_value, VALUE rb_manager) {
  xmlSecKeysMngrPtr keyManager = (xmlSecKeysMngrPtr)rb_manager;
  char *keyName, *keyData;
  unsigned int keyDataLength;
  xmlSecKeyPtr key;

  Check_Type(rb_key, T_STRING);
  Check_Type(rb_value, T_STRING);
  keyName = RSTRING_PTR(rb_key);
  keyData = RSTRING_PTR(rb_value);
  keyDataLength = RSTRING_LEN(rb_value);

  // load key
  key = xmlSecCryptoAppKeyLoadMemory((xmlSecByte *)keyData,
                                     keyDataLength,
                                     xmlSecKeyDataFormatPem,
                                     NULL, // password
                                     NULL, NULL);
  if (key == NULL) {
    rb_warn("failed to load '%s' public or private pem key", keyName);
    return ST_CONTINUE;
  }

  // set key name
  if (xmlSecKeySetName(key, BAD_CAST keyName) < 0) {
    rb_warn("failed to set key name for key '%s'", keyName);
    return ST_CONTINUE;
  }
  
  // add key to key manager; from now on the manager is responsible for
  // destroying the key
  if (xmlSecCryptoAppDefaultKeysMngrAdoptKey(keyManager, key) < 0) {
    rb_warn("failed to add key '%s' to key manager", keyName);
    return ST_CONTINUE;
  }

  return ST_CONTINUE;
}

static xmlSecKeysMngrPtr getKeyManager(VALUE rb_hash) {
  xmlSecKeysMngrPtr keyManager = xmlSecKeysMngrCreate();
  if (keyManager == NULL) return NULL;
  if (xmlSecCryptoAppDefaultKeysMngrInit(keyManager) < 0) {
    rb_raise(rb_eKeystoreError, "could not initialize key manager");
    xmlSecKeysMngrDestroy(keyManager);
    return NULL;
  }

  rb_hash_foreach(rb_hash, addRubyKeyToManager, (VALUE)keyManager);

  return keyManager;
}

VALUE verify_signature_with_named_keys(VALUE self, VALUE rb_hash) {
  xmlDocPtr doc;
  xmlNodePtr node = NULL;
  xmlSecDSigCtxPtr dsigCtx = NULL;
  xmlSecKeysMngrPtr keyManager = NULL;
  VALUE result = Qfalse;

  Check_Type(rb_hash, T_HASH);
  Data_Get_Struct(self, xmlDoc, doc);

  // find start node
  node = xmlSecFindNode(xmlDocGetRootElement(doc), xmlSecNodeSignature, xmlSecDSigNs);
  if(node == NULL) {
    rb_raise(rb_eVerificationError, "start node not found");
    goto done;
  }

  keyManager = getKeyManager(rb_hash);
  if (keyManager == NULL) {
    rb_raise(rb_eKeystoreError, "failed to create key manager");
    goto done;
  }

  // create signature context, we don't need keys manager in this example
  dsigCtx = xmlSecDSigCtxCreate(keyManager);
  if(dsigCtx == NULL) {
    rb_raise(rb_eVerificationError, "failed to create signature context");
    goto done;
  }

  // verify signature
  if(xmlSecDSigCtxVerify(dsigCtx, node) < 0) {
    rb_raise(rb_eVerificationError, "signature could not be verified");
    goto done;
  }
      
  if(dsigCtx->status == xmlSecDSigStatusSucceeded) {
    result = Qtrue;
  }    

done:
  if(dsigCtx != NULL) {
    xmlSecDSigCtxDestroy(dsigCtx);
  }

  if (keyManager != NULL) {
    xmlSecKeysMngrDestroy(keyManager);
  }

  return result;
}
