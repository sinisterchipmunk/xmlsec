#ifndef NOKOGIRI_EXT_XMLSEC_UTIL_H
#define NOKOGIRI_EXT_XMLSEC_UTIL_H

#include "xmlsecrb.h"

xmlSecKeysMngrPtr getKeyManager(char* keyStr, unsigned int keyLength,
                                char *keyName,
                                VALUE* rb_exception_result_out,
                                const char** exception_message_out);

#endif  // NOKOGIRI_EXT_XMLSEC_UTIL_H
