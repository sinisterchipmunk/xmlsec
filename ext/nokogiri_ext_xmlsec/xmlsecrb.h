#ifndef XMLSECRB_H
#define XMLSECRB_H

#include <ruby.h>

#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xmlstring.h>

#include <libxslt/xslt.h>

#include <xmlsec/xmlsec.h>
#include <xmlsec/xmltree.h>
#include <xmlsec/xmldsig.h>
#include <xmlsec/xmlenc.h>
#include <xmlsec/templates.h>
#include <xmlsec/crypto.h>
#include <xmlsec/dl.h>

VALUE sign_with_key(VALUE self, VALUE rb_key_name, VALUE rb_rsa_key);
VALUE sign_with_certificate(VALUE self, VALUE rb_key_name, VALUE rb_rsa_key, VALUE rb_cert);
VALUE verify_signature_with_rsa_key(VALUE self, VALUE rb_rsa_key);
VALUE verify_signature_with_named_keys(VALUE self, VALUE rb_keys);
VALUE verify_signature_with_certificates(VALUE self, VALUE rb_certs);
VALUE encrypt_with_key(VALUE self, VALUE rb_key_name, VALUE rb_key);
VALUE decrypt_with_key(VALUE self, VALUE rb_key_name, VALUE rb_key);
VALUE set_id_attribute(VALUE self, VALUE rb_attr_name);

void Init_Nokogiri_ext(void);

extern VALUE rb_cNokogiri_XML_Document;
extern VALUE rb_eSigningError;
extern VALUE rb_eVerificationError;
extern VALUE rb_eKeystoreError;
extern VALUE rb_eEncryptionError;
extern VALUE rb_eDecryptionError;

#endif // XMLSECRB_H
