#include "xmlsecrb.h"

static xmlSecKeysMngrPtr getKeyManager(VALUE rb_certs,
                                       VALUE* rb_exception_result_out,
                                       const char** exception_message_out) {
  VALUE rb_exception_result = Qnil;
  const char* exception_message = NULL;

  int i = 0;
  int numCerts = RARRAY_LEN(rb_certs);
  xmlSecKeysMngrPtr keyManager = xmlSecKeysMngrCreate();
  VALUE rb_cert = Qnil;
  char *cert = NULL;
  unsigned int certLength = 0;
  int numSuccessful = 0;

  if (keyManager == NULL) {
    rb_exception_result = rb_eDecryptionError;
    exception_message = "failed to create keys manager.";
    goto done;
  }

  if (xmlSecCryptoAppDefaultKeysMngrInit(keyManager) < 0) {
    rb_exception_result = rb_eKeystoreError;
    exception_message = "could not initialize key manager";
    goto done;
  }

  for (i = 0; i < numCerts; i++) {
    rb_cert = RARRAY_PTR(rb_certs)[i];
    Check_Type(rb_cert, T_STRING);
    cert = RSTRING_PTR(rb_cert);
    certLength = RSTRING_LEN(rb_cert);

    if(xmlSecCryptoAppKeysMngrCertLoadMemory(keyManager,
                                             (xmlSecByte *)cert,
                                             certLength,
                                             xmlSecKeyDataFormatPem,
                                             xmlSecKeyDataTypeTrusted) < 0) {
      rb_warn("failed to load certificate at index %d", i);
    } else {
      numSuccessful++;
    }
  }

  // note, numCerts could be zero, meaning that we should use system SSL certs
  if (numSuccessful == 0 && numCerts != 0) {
    rb_exception_result = rb_eKeystoreError;
    exception_message = "Could not load any of the specified certificates for signature verification";
    goto done;
  }

done:
  if (rb_exception_result != Qnil) {
    if (keyManager) {
      xmlSecKeysMngrDestroy(keyManager);
      keyManager = NULL;
    }
  }

  *rb_exception_result_out = rb_exception_result;
  *exception_message_out = exception_message;
  return keyManager;
}

VALUE verify_signature_with_certificates(VALUE self, VALUE rb_certs) {
  VALUE rb_exception_result = Qnil;
  const char* exception_message = NULL;

  xmlDocPtr doc = NULL;
  xmlNodePtr node = NULL;
  xmlSecDSigCtxPtr dsigCtx = NULL;
  xmlSecKeysMngrPtr keyManager = NULL;
  VALUE result = Qfalse;

  Check_Type(rb_certs, T_ARRAY);
  Data_Get_Struct(self, xmlDoc, doc);

  // find start node
  node = xmlSecFindNode(xmlDocGetRootElement(doc), xmlSecNodeSignature, xmlSecDSigNs);
  if(node == NULL) {
    rb_exception_result = rb_eVerificationError;
    exception_message = "start node not found";
    goto done;
  }

  keyManager = getKeyManager(rb_certs, &rb_exception_result,
                             &exception_message);
  if (keyManager == NULL) {
    // Propagate exception.
    goto done;
  }

  // create signature context, we don't need keys manager in this example
  dsigCtx = xmlSecDSigCtxCreate(keyManager);
  if(dsigCtx == NULL) {
    rb_exception_result = rb_eVerificationError;
    exception_message = "failed to create signature context";
    goto done;
  }

  // verify signature
  if(xmlSecDSigCtxVerify(dsigCtx, node) < 0) {
    rb_exception_result = rb_eVerificationError;
    exception_message = "error occurred during signature verification";
    goto done;
  }
      
  if(dsigCtx->status == xmlSecDSigStatusSucceeded) {
    result = Qtrue;
  }    

done:
  if(dsigCtx != NULL) {
    xmlSecDSigCtxDestroy(dsigCtx);
  }

  if (keyManager != NULL) {
    xmlSecKeysMngrDestroy(keyManager);
  }

  if(rb_exception_result != Qnil) {
    rb_raise(rb_exception_result, "%s", exception_message);
  }

  return result;
}
