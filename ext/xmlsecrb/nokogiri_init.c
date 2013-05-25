#include "xmlsecrb.h"

VALUE rb_cNokogiri_XML_Document = T_NIL;
VALUE rb_eSigningError = T_NIL;
VALUE rb_eVerificationError = T_NIL;
VALUE rb_eKeystoreError = T_NIL;

void Init_Nokogiri_ext() {
  VALUE XMLSec = rb_define_module("XMLSec");
  VALUE Nokogiri = rb_define_module("Nokogiri");
  VALUE Nokogiri_XML = rb_define_module_under(Nokogiri, "XML");
  rb_cNokogiri_XML_Document = rb_const_get(Nokogiri_XML, rb_intern("Document"));

  rb_define_method(rb_cNokogiri_XML_Document, "_sign_rsa", sign_rsa, 2);
  rb_define_method(rb_cNokogiri_XML_Document, "_sign_x509", sign_x509, 3);
  rb_define_method(rb_cNokogiri_XML_Document, "verify_with_rsa_key", verify_signature_with_rsa_key, 1);
  rb_define_method(rb_cNokogiri_XML_Document, "verify_with_named_keys", verify_signature_with_named_keys, 1);
  rb_define_method(rb_cNokogiri_XML_Document, "verify_with_certificates", verify_signature_with_certificates, 1);

  rb_eSigningError      = rb_define_class_under(XMLSec, "SigningError",      rb_eRuntimeError);
  rb_eVerificationError = rb_define_class_under(XMLSec, "VerificationError", rb_eRuntimeError);
  rb_eKeystoreError     = rb_define_class_under(XMLSec, "KeystoreError",     rb_eRuntimeError);
}
