#include "xmlsecrb.h"

VALUE sign_with_certificate(VALUE self, VALUE rb_key_name, VALUE rb_rsa_key, VALUE rb_cert) {
  VALUE rb_exception_result = Qnil;
  const char* exception_message = NULL;

  xmlDocPtr doc = NULL;
  xmlNodePtr signNode = NULL;
  xmlNodePtr refNode = NULL;
  xmlNodePtr keyInfoNode = NULL;
  xmlSecDSigCtxPtr dsigCtx = NULL;
  char *keyName = NULL;
  char *certificate = NULL;
  char *rsaKey = NULL;
  unsigned int rsaKeyLength = 0;
  unsigned int certificateLength = 0;

  Data_Get_Struct(self, xmlDoc, doc);

  Check_Type(rb_rsa_key, T_STRING);
  Check_Type(rb_key_name, T_STRING);
  Check_Type(rb_cert, T_STRING);

  rsaKey = RSTRING_PTR(rb_rsa_key);
  rsaKeyLength = RSTRING_LEN(rb_rsa_key);
  keyName = strndup(RSTRING_PTR(rb_key_name), RSTRING_LEN(rb_key_name) + 1);
  certificate = RSTRING_PTR(rb_cert);
  certificateLength = RSTRING_LEN(rb_cert);

  // create signature template for RSA-SHA1 enveloped signature
  signNode = xmlSecTmplSignatureCreate(doc, xmlSecTransformExclC14NId,
                                       xmlSecTransformRsaSha1Id, NULL);
  if (signNode == NULL) {
    rb_exception_result = rb_eSigningError;
    exception_message = "failed to create signature template";
    goto done;
  }

  // add <dsig:Signature/> node to the doc
  xmlAddChild(xmlDocGetRootElement(doc), signNode);

  // add reference
  refNode = xmlSecTmplSignatureAddReference(signNode, xmlSecTransformSha1Id,
                                            NULL,
                                            NULL,
                                            NULL);
  if(refNode == NULL) {
    rb_exception_result = rb_eSigningError;
    exception_message = "failed to add reference to signature template";
    goto done;
  }

  // add enveloped transform
  if(xmlSecTmplReferenceAddTransform(refNode, xmlSecTransformEnvelopedId) == NULL) {
    rb_exception_result = rb_eSigningError;
    exception_message = "failed to add enveloped transform to reference";
    goto done;
  }

  if(xmlSecTmplReferenceAddTransform(refNode, xmlSecTransformExclC14NId) == NULL) {
    rb_exception_result = rb_eSigningError;
    exception_message = "failed to add canonicalization transform to reference";
    goto done;
  }

  // add <dsig:KeyInfo/> and <dsig:X509Data/>
  keyInfoNode = xmlSecTmplSignatureEnsureKeyInfo(signNode, NULL);
  if(keyInfoNode == NULL) {
    rb_exception_result = rb_eSigningError;
    exception_message = "failed to add key info";
    goto done;
  }
  
  if(xmlSecTmplKeyInfoAddX509Data(keyInfoNode) == NULL) {
    rb_exception_result = rb_eSigningError;
    exception_message = "failed to add X509Data node";
    goto done;
  }

  // create signature context, we don't need keys manager in this example
  dsigCtx = xmlSecDSigCtxCreate(NULL);
  if(dsigCtx == NULL) {
    rb_exception_result = rb_eSigningError;
    exception_message = "failed to create signature context";
    goto done;
  }

  // load private key, assuming that there is not password
  dsigCtx->signKey = xmlSecCryptoAppKeyLoadMemory((xmlSecByte *)rsaKey,
                                                  rsaKeyLength,
                                                  xmlSecKeyDataFormatPem,
                                                  NULL, // password
                                                  NULL,
                                                  NULL);
  if(dsigCtx->signKey == NULL) {
    rb_exception_result = rb_eSigningError;
    exception_message = "failed to load private key";
    goto done;
  }
  
  // load certificate and add to the key
  if(xmlSecCryptoAppKeyCertLoadMemory(dsigCtx->signKey,
                                      (xmlSecByte *)certificate,
                                      certificateLength,
                                      xmlSecKeyDataFormatPem) < 0) {
    rb_exception_result = rb_eSigningError;
    exception_message = "failed to load certificate";
    goto done;
  }

  // set key name
  if(xmlSecKeySetName(dsigCtx->signKey, (xmlSecByte *)keyName) < 0) {
    rb_exception_result = rb_eSigningError;
    exception_message = "failed to set key name";
    goto done;
  }

  // sign the template
  if(xmlSecDSigCtxSign(dsigCtx, signNode) < 0) {
    rb_exception_result = rb_eSigningError;
    exception_message = "signature failed";
    goto done;
  }

done:
  if(dsigCtx != NULL) {
    xmlSecDSigCtxDestroy(dsigCtx);
  }

  free(keyName);

  if(rb_exception_result != Qnil) {
    rb_raise(rb_exception_result, "%s", exception_message);
  }

  return Qnil;
}
