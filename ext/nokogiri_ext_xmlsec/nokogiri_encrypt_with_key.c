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

VALUE encrypt_with_key(VALUE self, VALUE rb_key_name, VALUE rb_key) {
  xmlDocPtr doc;
  xmlNodePtr encDataNode = NULL;
  xmlNodePtr encKeyNode  = NULL;
  xmlNodePtr keyInfoNode = NULL;
  xmlSecEncCtxPtr encCtx = NULL;
  xmlSecKeysMngrPtr keyManager = NULL;
  char *keyName;
  char *key;
  unsigned int keyLength;

  Check_Type(rb_key_name, T_STRING);
  Check_Type(rb_key,      T_STRING);
  Data_Get_Struct(self, xmlDoc, doc);
  key       = RSTRING_PTR(rb_key);
  keyLength = RSTRING_LEN(rb_key);
  keyName   = RSTRING_PTR(rb_key_name);

  // create encryption template to encrypt XML file and replace 
  // its content with encryption result
  encDataNode = xmlSecTmplEncDataCreate(doc, xmlSecTransformDes3CbcId,
                              NULL, xmlSecTypeEncElement, NULL, NULL);
  if(encDataNode == NULL) {
    rb_raise(rb_eEncryptionError, "failed to create encryption template");
    goto done;
  }

  // we want to put encrypted data in the <enc:CipherValue/> node
  if(xmlSecTmplEncDataEnsureCipherValue(encDataNode) == NULL) {
    rb_raise(rb_eEncryptionError, "failed to add CipherValue node");
    goto done;
  }

  // add <dsig:KeyInfo/> and <dsig:KeyName/> nodes to put key name in the
  // signed document
  keyInfoNode = xmlSecTmplEncDataEnsureKeyInfo(encDataNode, NULL);
  if(keyInfoNode == NULL) {
    rb_raise(rb_eEncryptionError, "failed to add key info");
    goto done;
  }

  if(xmlSecTmplKeyInfoAddKeyName(keyInfoNode, NULL) == NULL) {
    rb_raise(rb_eEncryptionError, "failed to add key name");
    goto done;
  }

  keyManager = getKeyManager(key, keyLength, keyName);
  if (keyManager == NULL) {
    rb_raise(rb_eEncryptionError, "failed to create key manager");
    goto done;
  }

  // create encryption context, we don't need keys manager in this example
  encCtx = xmlSecEncCtxCreate(keyManager);
  if(encCtx == NULL) {
    rb_raise(rb_eEncryptionError, "failed to create encryption context");
    goto done;
  }

  // generate a 3DES key
  // TODO make a note of this one, it lets us pass in key type and bits from ruby
  encCtx->encKey = xmlSecKeyGenerateByName((xmlChar *)"des", 192,
                                           xmlSecKeyDataTypeSession);

  // encCtx->encKey = xmlSecKeyGenerate(xmlSecKeyDataDesId, 192,
  //                                    xmlSecKeyDataTypeSession);
  // encCtx->encKey = xmlSecAppCryptoKeyGenerate(xmlSecAppCmdLineParamGetString(&sessionKeyParam),
  //                               NULL, xmlSecKeyDataTypeSession);
  if(encCtx->encKey == NULL) {
    rb_raise(rb_eDecryptionError, "failed to generate session des key");
    goto done;
  }

  // set key name
  if(xmlSecKeySetName(encCtx->encKey, (xmlSecByte *)keyName) < 0) {
    rb_raise(rb_eEncryptionError, "failed to set key name to '%s'", keyName);
    goto done;
  }

  // add <enc:EncryptedKey/> node to the <dsig:KeyInfo/> tag to include
  // the session key
  encKeyNode = xmlSecTmplKeyInfoAddEncryptedKey(keyInfoNode,
                                       xmlSecTransformRsaPkcs1Id, // encMethodId encryptionMethod
                                       NULL, // xmlChar *idAttribute
                                       NULL, // xmlChar *typeAttribute
                                       NULL  // xmlChar *recipient
                                      );
  if (encKeyNode == NULL) {
    rb_raise(rb_eEncryptionError, "failed to add encrypted key node");
    goto done;
  }
  if (xmlSecTmplEncDataEnsureCipherValue(encKeyNode) == NULL) {
    rb_raise(rb_eEncryptionError, "failed to add encrypted cipher value");
    goto done;
  }

  // encrypt the data
  if(xmlSecEncCtxXmlEncrypt(encCtx, encDataNode, xmlDocGetRootElement(doc)) < 0) {
    rb_raise(rb_eEncryptionError, "encryption failed");
    goto done;
  }
  
  // the template is inserted in the doc, so don't free it
  encDataNode = NULL;
  encKeyNode = NULL;

done:

  /* cleanup */
  if(encCtx != NULL) {
    xmlSecEncCtxDestroy(encCtx);
  }

  if (encKeyNode != NULL) {
    xmlFreeNode(encKeyNode);
  }

  if(encDataNode != NULL) {
    xmlFreeNode(encDataNode);
  }

  if (keyManager != NULL) {
    xmlSecKeysMngrDestroy(keyManager);
  }

  return T_NIL;
}
