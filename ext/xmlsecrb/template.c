#include "xmlsecrb.h"


// // xmlNewDoc(BAD_CAST str);

// VALUE rb_cXMLSec_Template = T_NIL;

// static VALUE sign(VALUE self, VALUE rb_cert) {
  
//   xmlDocPtr doc = NULL;
//   xmlNodePtr node = NULL;
//   xmlSecDSigCtxPtr dsigCtx = NULL;
//   int certLen;
//   char *cert, *xml;

//   Check_Type(rb_cert, T_STRING);
//   certLen = RSTRING_LEN(rb_cert);
//   cert = RSTRING_PTR(rb_cert);
//   xml  = RSTRING_PTR(rb_funcall(self, rb_intern("to_s"), 0));

//   // load template
//   doc = xmlNewDoc(xml);
//   printf(xml);
//   if ((doc == NULL) || (xmlDocGetRootElement(doc) == NULL)) {
//     rb_raise(rb_eRuntimeError, "unable to parse document");
//     goto done;
//   }

//   // find start node
//   node = xmlSecFindNode(xmlDocGetRootElement(doc), xmlSecNodeSignature, xmlSecDSigNs);
//   if(node == NULL) {
//     rb_raise(rb_eRuntimeError, "start node not found");
//     goto done;  
//   }

//   // create signature context
//   dsigCtx = xmlSecDSigCtxCreate(NULL);
//   if (dsigCtx == NULL) {
//     rb_raise(rb_eRuntimeError, "failed to create signature context");
//     goto done;
//   }

//   // load private key
//   dsigCtx->signKey = xmlSecCryptoAppKeyLoadMemory(
//     cert,
//     certLen,
//     xmlSecKeyDataFormatPem,
//     NULL,
//     NULL,
//     NULL
//   );

// done:
//   // cleanup
//   if (dsigCtx != NULL) {
//     xmlSecDSigCtxDestroy(dsigCtx);
//   }

//   if (doc != NULL) {
//     xmlFreeDoc(doc);
//   }
  
//   return T_NIL;
// }

// void Init_xmlsecrb_Template() {
//   rb_cXMLSec_Template = rb_define_class_under(rb_mXMLSec, "Template", rb_cObject);

//   rb_define_method(rb_cXMLSec_Template, "sign!", sign, 1);
// }
