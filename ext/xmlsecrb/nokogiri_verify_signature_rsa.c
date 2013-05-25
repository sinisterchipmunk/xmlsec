#include "xmlsecrb.h"

VALUE verify_signature_with_rsa_key(VALUE self, VALUE rb_rsa_key) {
  xmlDocPtr doc;
  xmlNodePtr node = NULL;
  xmlSecDSigCtxPtr dsigCtx = NULL;
  char *rsaKey;
  unsigned int rsaKeyLength;
  VALUE result = Qfalse;

  Data_Get_Struct(self, xmlDoc, doc);
  rsaKey = RSTRING_PTR(rb_rsa_key);
  rsaKeyLength = RSTRING_LEN(rb_rsa_key);

  // find start node
  node = xmlSecFindNode(xmlDocGetRootElement(doc), xmlSecNodeSignature, xmlSecDSigNs);
  if(node == NULL) {
    rb_raise(rb_eVerificationError, "start node not found");
    goto done;
  }

  // create signature context, we don't need keys manager in this example
  dsigCtx = xmlSecDSigCtxCreate(NULL);
  if(dsigCtx == NULL) {
    rb_raise(rb_eVerificationError, "failed to create signature context");
    goto done;
  }

  // load public key
  dsigCtx->signKey = xmlSecCryptoAppKeyLoadMemory((xmlSecByte *)rsaKey,
                                                  rsaKeyLength,
                                                  xmlSecKeyDataFormatPem,
                                                  NULL, // password
                                                  NULL, NULL);
  if(dsigCtx->signKey == NULL) {
    rb_raise(rb_eVerificationError, "failed to load public pem key");
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

  return result;
}
