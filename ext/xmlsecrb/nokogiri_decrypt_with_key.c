#include "xmlsecrb.h"

static xmlSecKeysMngrPtr getKeyManager(char* keyStr, unsigned int keyLength, char *keyName) {
  xmlSecKeysMngrPtr mngr;
  xmlSecKeyPtr key;
  
  /* create and initialize keys manager, we use a simple list based
   * keys manager, implement your own xmlSecKeysStore klass if you need
   * something more sophisticated 
   */
  mngr = xmlSecKeysMngrCreate();
  if(mngr == NULL) {
    rb_raise(rb_eDecryptionError, "failed to create keys manager.");
    return(NULL);
  }
  if(xmlSecCryptoAppDefaultKeysMngrInit(mngr) < 0) {
    rb_raise(rb_eDecryptionError, "failed to initialize keys manager.");
    xmlSecKeysMngrDestroy(mngr);
    return(NULL);
  }    
  
  /* load private RSA key */
  // key = xmlSecCryptoAppKeyLoad(key_file, xmlSecKeyDataFormatPem, NULL, NULL, NULL);
  key = xmlSecCryptoAppKeyLoadMemory((xmlSecByte *)keyStr,
                                     keyLength,
                                     xmlSecKeyDataFormatPem,
                                     NULL, // password
                                     NULL, NULL);
  if(key == NULL) {
    rb_raise(rb_eDecryptionError, "failed to load rsa key");
    xmlSecKeysMngrDestroy(mngr);
    return(NULL);
  }

  /* set key name to the file name, this is just an example! */
  if(xmlSecKeySetName(key, BAD_CAST keyName) < 0) {
    rb_raise(rb_eDecryptionError, "failed to set key name");
    xmlSecKeyDestroy(key);  
    xmlSecKeysMngrDestroy(mngr);
    return(NULL);
  }

  /* add key to keys manager, from now on keys manager is responsible 
   * for destroying key 
   */
  if(xmlSecCryptoAppDefaultKeysMngrAdoptKey(mngr, key) < 0) {
    rb_raise(rb_eDecryptionError, "failed to add key to keys manager");
    xmlSecKeyDestroy(key);
    xmlSecKeysMngrDestroy(mngr);
    return(NULL);
  }

  return(mngr);
}

VALUE decrypt_with_key(VALUE self, VALUE rb_key_name, VALUE rb_key) {
  xmlDocPtr doc;
  xmlNodePtr node = NULL;
  xmlSecEncCtxPtr encCtx = NULL;
  xmlSecKeysMngrPtr keyManager = NULL;
  char *key;
  char *keyName;
  unsigned int keyLength;

  Check_Type(rb_key,      T_STRING);
  Check_Type(rb_key_name, T_STRING);
  Data_Get_Struct(self, xmlDoc, doc);
  key       = RSTRING_PTR(rb_key);
  keyLength = RSTRING_LEN(rb_key);
  keyName   = RSTRING_PTR(rb_key_name);


  // find start node
  node = xmlSecFindNode(xmlDocGetRootElement(doc), xmlSecNodeEncryptedData, xmlSecEncNs);
  if(node == NULL) {
      rb_raise(rb_eDecryptionError, "start node not found");
      goto done;      
  }

  keyManager = getKeyManager(key, keyLength, keyName);
  if (keyManager == NULL) {
    rb_raise(rb_eEncryptionError, "failed to create key manager");
    goto done;
  }

  // create encryption context
  encCtx = xmlSecEncCtxCreate(keyManager);
  if(encCtx == NULL) {
    rb_raise(rb_eDecryptionError, "failed to create encryption context");
    goto done;
  }

  // decrypt the data
  if((xmlSecEncCtxDecrypt(encCtx, node) < 0) || (encCtx->result == NULL)) {
    rb_raise(rb_eDecryptionError, "decryption failed");
    goto done;
  }

  if(encCtx->resultReplaced == 0) {
    fprintf(stdout, "Decrypted binary data (%d bytes):", xmlSecBufferGetSize(encCtx->result));
    if(xmlSecBufferGetData(encCtx->result) != NULL) {
      fwrite(xmlSecBufferGetData(encCtx->result), 
             1, 
             xmlSecBufferGetSize(encCtx->result),
             stdout);
      fprintf(stdout, "\n");
    }

    rb_raise(rb_eDecryptionError, "Not implemented: don't know how to handle decrypted, non-XML data yet");
    goto done;
  }

done:    
  // cleanup
  if(encCtx != NULL) {
    xmlSecEncCtxDestroy(encCtx);
  }
  
  if (keyManager != NULL) {
    xmlSecKeysMngrDestroy(keyManager);
  }

  return T_NIL;
}
