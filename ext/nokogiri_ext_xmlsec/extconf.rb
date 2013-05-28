require 'mkmf'

def barf message = 'dependencies not met'
  raise message
end

barf unless have_header('ruby.h')

if pkg_config('xmlsec1-openssl')
  # HACK 'openssl' is escaped too many times, I don't know why
  if $CFLAGS =~ /\-DXMLSEC_CRYPTO=\\\\\\"openssl\\\\\\"/
    $CFLAGS['-DXMLSEC_CRYPTO=\\\\\\"openssl\\\\\\"'] =
      '-DXMLSEC_CRYPTO=\\"openssl\\"'
  end
  
  have_library 'xmlsec1-openssl'
  create_makefile('nokogiri_ext_xmlsec')
else
  barf "xmlsec1 is not installed."
end
