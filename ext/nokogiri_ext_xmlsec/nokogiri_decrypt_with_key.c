#include "xmlsecrb.h"
#include "util.h"

VALUE decrypt_with_key(VALUE self, VALUE rb_key_name, VALUE rb_key) {
  VALUE rb_exception_result = Qnil;
  const char* exception_message = NULL;

  xmlDocPtr doc = NULL;
  xmlNodePtr node = NULL;
  xmlSecEncCtxPtr encCtx = NULL;
  xmlSecKeysMngrPtr keyManager = NULL;
  char *key = NULL;
  char *keyName = NULL;
  unsigned int keyLength = 0;

  Check_Type(rb_key,      T_STRING);
  Check_Type(rb_key_name, T_STRING);
  Data_Get_Struct(self, xmlDoc, doc);
  key       = RSTRING_PTR(rb_key);
  keyLength = RSTRING_LEN(rb_key);
  keyName = strndup(RSTRING_PTR(rb_key_name), RSTRING_LEN(rb_key_name) + 1);

  // find start node
  node = xmlSecFindNode(xmlDocGetRootElement(doc), xmlSecNodeEncryptedData, xmlSecEncNs);
  if(node == NULL) {
      rb_exception_result = rb_eDecryptionError;
      exception_message = "start node not found";
      goto done;      
  }

  keyManager = getKeyManager(key, keyLength, keyName, &rb_exception_result,
                             &exception_message);
  if (keyManager == NULL) {
    // Propagate the exception.
    goto done;
  }

  // create encryption context
  encCtx = xmlSecEncCtxCreate(keyManager);
  if(encCtx == NULL) {
    rb_exception_result = rb_eDecryptionError;
    exception_message = "failed to create encryption context";
    goto done;
  }

  // decrypt the data
  if((xmlSecEncCtxDecrypt(encCtx, node) < 0) || (encCtx->result == NULL)) {
    rb_exception_result = rb_eDecryptionError;
    exception_message = "decryption failed";
    goto done;
  }

  if(encCtx->resultReplaced == 0) {
    rb_warn("Decrypted binary data (%d bytes):", xmlSecBufferGetSize(encCtx->result));

    rb_exception_result = rb_eDecryptionError;
    exception_message =  "Not implemented: don't know how to handle decrypted, non-XML data yet";
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

  free(keyName);

  if(rb_exception_result != Qnil) {
    rb_raise(rb_exception_result, "%s", exception_message);
  }

  return Qnil;
}
