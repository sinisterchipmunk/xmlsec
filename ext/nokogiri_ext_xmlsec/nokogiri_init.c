#include "xmlsecrb.h"

VALUE rb_cNokogiri_XML_Document = Qnil;
VALUE rb_cNokogiri_XML_Node = Qnil;
VALUE rb_eSigningError = Qnil;
VALUE rb_eVerificationError = Qnil;
VALUE rb_eKeystoreError = Qnil;
VALUE rb_eEncryptionError = Qnil;
VALUE rb_eDecryptionError = Qnil;

void Init_Nokogiri_ext() {
  VALUE XMLSec = rb_define_module("XMLSec");
  VALUE Nokogiri = rb_define_module("Nokogiri");
  VALUE Nokogiri_XML = rb_define_module_under(Nokogiri, "XML");
  rb_cNokogiri_XML_Document = rb_const_get(Nokogiri_XML, rb_intern("Document"));
  rb_cNokogiri_XML_Node = rb_const_get(Nokogiri_XML, rb_intern("Node"));

  rb_define_method(rb_cNokogiri_XML_Document, "sign_with_key",            sign_with_key, 2);
  rb_define_method(rb_cNokogiri_XML_Document, "sign_with_certificate",    sign_with_certificate, 3);
  rb_define_method(rb_cNokogiri_XML_Document, "verify_with_rsa_key",      verify_signature_with_rsa_key, 1);
  rb_define_method(rb_cNokogiri_XML_Document, "verify_with_named_keys",   verify_signature_with_named_keys, 1);
  rb_define_method(rb_cNokogiri_XML_Document, "verify_with_certificates", verify_signature_with_certificates, 1);
  rb_define_method(rb_cNokogiri_XML_Document, "encrypt_with_key",         encrypt_with_key, 2);
  rb_define_method(rb_cNokogiri_XML_Document, "decrypt_with_key",         decrypt_with_key, 2);
  rb_define_method(rb_cNokogiri_XML_Node,     "set_id_attribute",         set_id_attribute, 1);

  rb_eSigningError      = rb_define_class_under(XMLSec, "SigningError",      rb_eRuntimeError);
  rb_eVerificationError = rb_define_class_under(XMLSec, "VerificationError", rb_eRuntimeError);
  rb_eKeystoreError     = rb_define_class_under(XMLSec, "KeystoreError",     rb_eRuntimeError);
  rb_eEncryptionError   = rb_define_class_under(XMLSec, "EncryptionError",   rb_eRuntimeError);
  rb_eDecryptionError   = rb_define_class_under(XMLSec, "DecryptionError",   rb_eRuntimeError);
}
