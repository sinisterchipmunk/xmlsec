#include "xmlsecrb.h"

VALUE verify_signature_with_rsa_key(VALUE self, VALUE rb_rsa_key) {
  VALUE rb_exception_result = Qnil;
  const char* exception_message = NULL;

  xmlDocPtr doc = NULL;
  xmlNodePtr node = NULL;
  xmlSecDSigCtxPtr dsigCtx = NULL;
  char *rsa_key = NULL;
  unsigned int rsa_key_length = 0;
  VALUE result = Qfalse;

  Data_Get_Struct(self, xmlDoc, doc);
  Check_Type(rb_rsa_key,  T_STRING);
  rsa_key = RSTRING_PTR(rb_rsa_key);
  rsa_key_length = RSTRING_LEN(rb_rsa_key);

  // find start node
  node = xmlSecFindNode(xmlDocGetRootElement(doc), xmlSecNodeSignature, xmlSecDSigNs);
  if(node == NULL) {
    rb_exception_result = rb_eVerificationError;
    exception_message = "start node not found";
    goto done;
  }

  // create signature context, we don't need keys manager in this example
  dsigCtx = xmlSecDSigCtxCreate(NULL);
  if(dsigCtx == NULL) {
    rb_exception_result = rb_eVerificationError;
    exception_message = "failed to create signature context";
    goto done;
  }

  // load public key
  dsigCtx->signKey = xmlSecCryptoAppKeyLoadMemory((xmlSecByte *)rsa_key,
                                                  rsa_key_length,
                                                  xmlSecKeyDataFormatPem,
                                                  NULL, // password
                                                  NULL, NULL);
  if(dsigCtx->signKey == NULL) {
    rb_exception_result = rb_eVerificationError;
    exception_message = "failed to load public pem key";
    goto done;
  }

  // verify signature
  if(xmlSecDSigCtxVerify(dsigCtx, node) < 0) {
    rb_exception_result = rb_eVerificationError;
    exception_message = "signature could not be verified";
    goto done;
  }
      
  if(dsigCtx->status == xmlSecDSigStatusSucceeded) {
    result = Qtrue;
  }    

done:
  if(dsigCtx != NULL) {
    xmlSecDSigCtxDestroy(dsigCtx);
  }

  if(rb_exception_result != Qnil) {
    rb_raise(rb_exception_result, "%s", exception_message);
  }

  return result;
}
