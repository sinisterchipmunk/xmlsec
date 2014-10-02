#include "xmlsecrb.h"
#include "util.h"

VALUE encrypt_with_key(VALUE self, VALUE rb_key_name, VALUE rb_key) {
  VALUE rb_exception_result = Qnil;
  const char* exception_message = NULL;

  xmlDocPtr doc = NULL;
  xmlNodePtr encDataNode = NULL;
  xmlNodePtr encKeyNode  = NULL;
  xmlNodePtr keyInfoNode = NULL;
  xmlSecEncCtxPtr encCtx = NULL;
  xmlSecKeysMngrPtr keyManager = NULL;
  char *keyName = NULL;
  char *key = NULL;
  unsigned int keyLength = 0;

  Check_Type(rb_key,      T_STRING);
  Check_Type(rb_key_name, T_STRING);
  Data_Get_Struct(self, xmlDoc, doc);
  key       = RSTRING_PTR(rb_key);
  keyLength = RSTRING_LEN(rb_key);
  keyName = strndup(RSTRING_PTR(rb_key_name), RSTRING_LEN(rb_key_name) + 1);

  // create encryption template to encrypt XML file and replace 
  // its content with encryption result
  encDataNode = xmlSecTmplEncDataCreate(doc, xmlSecTransformDes3CbcId,
                                        NULL, xmlSecTypeEncElement, NULL, NULL);
  if(encDataNode == NULL) {
    rb_exception_result = rb_eEncryptionError;
    exception_message = "failed to create encryption template";
    goto done;
  }

  // we want to put encrypted data in the <enc:CipherValue/> node
  if(xmlSecTmplEncDataEnsureCipherValue(encDataNode) == NULL) {
    rb_exception_result = rb_eEncryptionError;
    exception_message = "failed to add CipherValue node";
    goto done;
  }

  // add <dsig:KeyInfo/> and <dsig:KeyName/> nodes to put key name in the
  // signed document
  keyInfoNode = xmlSecTmplEncDataEnsureKeyInfo(encDataNode, NULL);
  if(keyInfoNode == NULL) {
    rb_exception_result = rb_eEncryptionError;
    exception_message = "failed to add key info";
    goto done;
  }

  if(xmlSecTmplKeyInfoAddKeyName(keyInfoNode, NULL) == NULL) {
    rb_exception_result = rb_eEncryptionError;
    exception_message = "failed to add key name";
    goto done;
  }

  keyManager = getKeyManager(key, keyLength, keyName, &rb_exception_result,
                             &exception_message);
  if (keyManager == NULL) {
    // Propagate the exception.
    goto done;
  }

  // create encryption context, we don't need keys manager in this example
  encCtx = xmlSecEncCtxCreate(keyManager);
  if(encCtx == NULL) {
    rb_exception_result = rb_eEncryptionError;
    exception_message = "failed to create encryption context";
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
    rb_exception_result = rb_eDecryptionError;
    exception_message = "failed to generate session des key";
    goto done;
  }

  // set key name
  if(xmlSecKeySetName(encCtx->encKey, (xmlSecByte *)keyName) < 0) {
    rb_exception_result = rb_eEncryptionError;
    exception_message = "failed to set key name";
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
    rb_exception_result = rb_eEncryptionError;
    exception_message = "failed to add encrypted key node";
    goto done;
  }
  if (xmlSecTmplEncDataEnsureCipherValue(encKeyNode) == NULL) {
    rb_exception_result = rb_eEncryptionError;
    exception_message = "failed to add encrypted cipher value";
    goto done;
  }

  // encrypt the data
  if(xmlSecEncCtxXmlEncrypt(encCtx, encDataNode, xmlDocGetRootElement(doc)) < 0) {
    rb_exception_result = rb_eEncryptionError;
    exception_message = "encryption failed";
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

  free(keyName);

  if(rb_exception_result != Qnil) {
    rb_raise(rb_exception_result, "%s", exception_message);
  }

  return Qnil;
}
