#include "util.h"

xmlSecKeysMngrPtr getKeyManager(char* keyStr, unsigned int keyLength,
                                char *keyName,
                                VALUE* rb_exception_result_out,
                                const char** exception_message_out) {
  VALUE rb_exception_result = Qnil;
  const char* exception_message = NULL;
  xmlSecKeysMngrPtr mngr = NULL;
  xmlSecKeyPtr key = NULL;
  
  /* create and initialize keys manager, we use a simple list based
   * keys manager, implement your own xmlSecKeysStore klass if you need
   * something more sophisticated 
   */
  mngr = xmlSecKeysMngrCreate();
  if(mngr == NULL) {
    rb_exception_result = rb_eDecryptionError;
    exception_message = "failed to create keys manager.";
    goto done;
  }
  if(xmlSecCryptoAppDefaultKeysMngrInit(mngr) < 0) {
    rb_exception_result = rb_eDecryptionError;
    exception_message = "failed to initialize keys manager.";
    goto done;
  }    
  
  /* load private RSA key */
  // key = xmlSecCryptoAppKeyLoad(key_file, xmlSecKeyDataFormatPem, NULL, NULL, NULL);
  key = xmlSecCryptoAppKeyLoadMemory((xmlSecByte *)keyStr,
                                     keyLength,
                                     xmlSecKeyDataFormatPem,
                                     NULL, // password
                                     NULL, NULL);
  if(key == NULL) {
    rb_exception_result = rb_eDecryptionError;
    exception_message = "failed to load rsa key";
    goto done;
  }

  /* set key name to the file name, this is just an example! */
  if(xmlSecKeySetName(key, BAD_CAST keyName) < 0) {
    rb_exception_result = rb_eDecryptionError;
    exception_message = "failed to set key name";
    goto done;
  }

  /* add key to keys manager, from now on keys manager is responsible 
   * for destroying key 
   */
  if(xmlSecCryptoAppDefaultKeysMngrAdoptKey(mngr, key) < 0) {
    rb_exception_result = rb_eDecryptionError;
    exception_message = "failed to add key to keys manager";
    goto done;
  }

done:
  if(rb_exception_result != Qnil) {
    if (key) {
      xmlSecKeyDestroy(key);
    }

    if (mngr) {
      xmlSecKeysMngrDestroy(mngr);
      mngr = NULL;
    }
  }

  *rb_exception_result_out = rb_exception_result;
  *exception_message_out = exception_message;
  return mngr;
}
