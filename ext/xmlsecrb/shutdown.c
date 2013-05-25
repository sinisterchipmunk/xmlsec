#include "xmlsecrb.h"

/* not actually called anywhere right now, but here for posterity */
void Shutdown_xmlsecrb() {
  xmlSecCryptoShutdown();
  xmlSecCryptoAppShutdown();
  xmlSecShutdown();
  xsltCleanupGlobals();
  #ifndef XMLSEC_NO_XSLT
    xsltCleanupGlobals();            
  #endif /* XMLSEC_NO_XSLT */
}
