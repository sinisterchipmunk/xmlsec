#include "xmlsecrb.h"

// TODO the signature context probably should be a ruby instance variable
// and separate object, instead of being allocated/freed in each method.

VALUE sign_with_key(VALUE self, VALUE rb_key_name, VALUE rb_rsa_key) {
  xmlDocPtr doc;
  xmlNodePtr signNode = NULL;
  xmlNodePtr refNode = NULL;
  xmlNodePtr keyInfoNode = NULL;
  xmlSecDSigCtxPtr dsigCtx = NULL;
  char *keyName;
  char *rsaKey;
  unsigned int rsaKeyLength;

  Data_Get_Struct(self, xmlDoc, doc);
  rsaKey = RSTRING_PTR(rb_rsa_key);
  rsaKeyLength = RSTRING_LEN(rb_rsa_key);
  keyName = RSTRING_PTR(rb_key_name);

  // create signature template for RSA-SHA1 enveloped signature
  signNode = xmlSecTmplSignatureCreate(doc, xmlSecTransformExclC14NId,
                                         xmlSecTransformRsaSha1Id, NULL);
  if (signNode == NULL) {
    rb_raise(rb_eSigningError, "failed to create signature template");
    goto done;
  }

  // add <dsig:Signature/> node to the doc
  xmlAddChild(xmlDocGetRootElement(doc), signNode);

  // add reference
  refNode = xmlSecTmplSignatureAddReference(signNode, xmlSecTransformSha1Id,
                                        NULL, NULL, NULL);
  if(refNode == NULL) {
    rb_raise(rb_eSigningError, "failed to add reference to signature template");
    goto done;
  }

  // add enveloped transform
  if(xmlSecTmplReferenceAddTransform(refNode, xmlSecTransformEnvelopedId) == NULL) {
    rb_raise(rb_eSigningError, "failed to add enveloped transform to reference");
    goto done;
  }

  // add <dsig:KeyInfo/> and <dsig:KeyName/> nodes to put key name in the signed
  // document
  keyInfoNode = xmlSecTmplSignatureEnsureKeyInfo(signNode, NULL);
  if(keyInfoNode == NULL) {
    rb_raise(rb_eSigningError, "failed to add key info");
    goto done;
  }
  if(xmlSecTmplKeyInfoAddKeyName(keyInfoNode, NULL) == NULL) {
    rb_raise(rb_eSigningError, "failed to add key name");
    goto done;
  }

  // create signature context, we don't need keys manager in this example
  dsigCtx = xmlSecDSigCtxCreate(NULL);
  if(dsigCtx == NULL) {
    rb_raise(rb_eSigningError, "failed to create signature context");
    goto done;
  }

  // load private key, assuming that there is no password
  dsigCtx->signKey = xmlSecCryptoAppKeyLoadMemory((xmlSecByte *)rsaKey,
                                                  rsaKeyLength,
                                                  xmlSecKeyDataFormatPem,
                                                  NULL, // password
                                                  NULL,
                                                  NULL);
  if(dsigCtx->signKey == NULL) {
    rb_raise(rb_eSigningError, "failed to load private pem key");
    goto done;
  }

  // set key name
  if(xmlSecKeySetName(dsigCtx->signKey, (xmlSecByte *)keyName) < 0) {
    rb_raise(rb_eSigningError, "failed to set key name");
    goto done;
  }

  // sign the template
  if(xmlSecDSigCtxSign(dsigCtx, signNode) < 0) {
    rb_raise(rb_eSigningError, "signature failed");
    goto done;
  }

done:
  if(dsigCtx != NULL) {
    xmlSecDSigCtxDestroy(dsigCtx);
  }

  return T_NIL;
}
