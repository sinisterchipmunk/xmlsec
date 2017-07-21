#include "xmlsecrb.h"

VALUE sign_with_certificate(VALUE self, VALUE rb_key_name, VALUE rb_rsa_key, VALUE rb_cert) {
  xmlDocPtr doc;
  xmlNodePtr signNode = NULL;
  xmlNodePtr refNode = NULL;
  xmlNodePtr keyInfoNode = NULL;
  xmlNodePtr x509Node = NULL;
  xmlNodePtr x509SerialNode = NULL;
  xmlSecDSigCtxPtr dsigCtx = NULL;
  char *keyName;
  char *certificate;
  char *rsaKey;
  unsigned int rsaKeyLength, certificateLength;

  Data_Get_Struct(self, xmlDoc, doc);
  rsaKey = RSTRING_PTR(rb_rsa_key);
  rsaKeyLength = RSTRING_LEN(rb_rsa_key);
  keyName = RSTRING_PTR(rb_key_name);
  certificate = RSTRING_PTR(rb_cert);
  certificateLength = RSTRING_LEN(rb_cert);

  // create signature template for RSA-SHA1 enveloped signature
  signNode = xmlSecTmplSignatureCreateNsPref(doc, xmlSecTransformInclC14NId,
                                         xmlSecTransformRsaSha1Id, NULL, "ds");
  if (signNode == NULL) {
    rb_raise(rb_eSigningError, "failed to create signature template");
    goto done;
  }

  // add <dsig:Signature/> node to the doc
  xmlAddChild(xmlDocGetRootElement(doc), signNode);

  // add reference
  refNode = xmlSecTmplSignatureAddReference(signNode, xmlSecTransformSha1Id,
                                        NULL, "", NULL);
  if(refNode == NULL) {
    rb_raise(rb_eSigningError, "failed to add reference to signature template");
    goto done;
  }

  // add enveloped transform
  if(xmlSecTmplReferenceAddTransform(refNode, xmlSecTransformEnvelopedId) == NULL) {
    rb_raise(rb_eSigningError, "failed to add enveloped transform to reference");
    goto done;
  }

  // add enveloped transform
    if(xmlSecTmplReferenceAddTransform(refNode, xmlSecTransformInclC14NId) == NULL) {
      rb_raise(rb_eSigningError, "failed to add enveloped transform to reference");
      goto done;
    }

  // add <dsig:KeyInfo/> and <dsig:X509Data/>
  keyInfoNode = xmlSecTmplSignatureEnsureKeyInfo(signNode, NULL);
  if(keyInfoNode == NULL) {
    rb_raise(rb_eSigningError, "failed to add key info");
    goto done;
  }

  x509Node = xmlSecTmplKeyInfoAddX509Data(keyInfoNode);
  if( x509Node == NULL) {
    rb_raise(rb_eSigningError, "failed to add X509Data node");
    goto done;
  }

  x509SerialNode = xmlSecTmplX509DataAddIssuerSerial(x509Node);
  xmlSecTmplX509IssuerSerialAddIssuerName(x509SerialNode, "CN=Gandi Standard SSL CA 2,O=Gandi,L=Paris,ST=Paris,C=FR");
  xmlSecTmplX509IssuerSerialAddSerialNumber(x509SerialNode, "180768426381044633091722485386902989652");

  // create signature context, we don't need keys manager in this example
  dsigCtx = xmlSecDSigCtxCreate(NULL);
  if(dsigCtx == NULL) {
    rb_raise(rb_eSigningError, "failed to create signature context");
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
    rb_raise(rb_eSigningError, "failed to load private key");
    goto done;
  }
  
  // load certificate and add to the key
  if(xmlSecCryptoAppKeyCertLoadMemory(dsigCtx->signKey,
                                      (xmlSecByte *)certificate,
                                      certificateLength,
                                      xmlSecKeyDataFormatPem) < 0) {
    rb_raise(rb_eSigningError, "failed to load certificate");
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
