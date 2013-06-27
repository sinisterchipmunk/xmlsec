#include "xmlsecrb.h"

static xmlSecKeysMngrPtr getKeyManager(VALUE rb_certs) {
  int i, numCerts = RARRAY_LEN(rb_certs);
  xmlSecKeysMngrPtr keyManager = xmlSecKeysMngrCreate();
  VALUE rb_cert;
  char *cert;
  unsigned int certLength;
  int numSuccessful = 0;

  if (keyManager == NULL) return NULL;

  if (xmlSecCryptoAppDefaultKeysMngrInit(keyManager) < 0) {
    rb_raise(rb_eKeystoreError, "could not initialize key manager");
    xmlSecKeysMngrDestroy(keyManager);
    return NULL;
  }

  for (i = 0; i < numCerts; i++) {
    rb_cert = RARRAY_PTR(rb_certs)[i];
    Check_Type(rb_cert, T_STRING);
    cert = RSTRING_PTR(rb_cert);
    certLength = RSTRING_LEN(rb_cert);

    if(xmlSecCryptoAppKeysMngrCertLoadMemory(keyManager,
                                             (xmlSecByte *)cert,
                                             certLength,
                                             xmlSecKeyDataFormatPem,
                                             xmlSecKeyDataTypeTrusted) < 0) {
      rb_warn("failed to load certificate at index %d", i);
    } else {
      numSuccessful++;
    }
  }

  // note, numCerts could be zero, meaning that we should use system SSL certs
  if (numSuccessful == 0 && numCerts != 0) {
    rb_raise(rb_eKeystoreError, "Could not load any of the specified certificates for signature verification");
    xmlSecKeysMngrDestroy(keyManager);
    return NULL;
  }

  return keyManager;
}

VALUE verify_signature_with_certificates(VALUE self, VALUE rb_certs) {
  xmlDocPtr doc;
  xmlNodePtr node = NULL;
  xmlSecDSigCtxPtr dsigCtx = NULL;
  xmlSecKeysMngrPtr keyManager = NULL;
  VALUE result = Qfalse;

  Check_Type(rb_certs, T_ARRAY);
  Data_Get_Struct(self, xmlDoc, doc);

  // find start node
  node = xmlSecFindNode(xmlDocGetRootElement(doc), xmlSecNodeSignature, xmlSecDSigNs);
  if(node == NULL) {
    rb_raise(rb_eVerificationError, "start node not found");
    goto done;
  }

  keyManager = getKeyManager(rb_certs);
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
    rb_raise(rb_eVerificationError, "error occurred during signature verification");
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
