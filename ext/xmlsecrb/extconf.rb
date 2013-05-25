require 'mkmf'

def barf
  raise 'dependencies not met'
end

barf unless have_header('ruby.h')

if pkg_config('xmlsec1-openssl')
  # HACK 'openssl' is escaped too many times, I don't know why
  if $CFLAGS =~ /\-DXMLSEC_CRYPTO=\\\\\\"openssl\\\\\\"/
    $CFLAGS['-DXMLSEC_CRYPTO=\\\\\\"openssl\\\\\\"'] =
      '-DXMLSEC_CRYPTO=\\"openssl\\"'
  end
  
  have_library 'xmlsec1-openssl'
  create_makefile('xmlsecrb')
else
  puts "xmlsec1 is not installed."
end


# barf unless find_executable('xmlsec1-config')

# $CFLAGS     = ENV["CFLAGS"].to_s + " " + `xmlsec1-config --cflags`.chomp
# $LOCAL_LIBS = ENV["LIBS"].to_s   + " " + `xmlsec1-config --libs`.chomp

# # barf unless have_library('xml2')#, 'xmlBufferCreate')
# # barf unless have_library('xslt')
# # barf unless have_library('xmlsec1')#, 'xmlSecInit')
# # barf unless find_header('openssl/ssl.h')
# # barf unless find_header('libxml/tree.h', '/usr/include/libxml2')
# # barf unless find_header('libxslt/xslt.h')
# # barf unless find_header('xmlsec/xmlsec.h', '/usr/include/xmlsec1')

# # barf unless have_library('openssl', 'SSL_load_error_strings')
# # barf unless have_library('xml2')#, 'xmlBufferCreate')
# # barf unless have_library('xslt')
# # barf unless have_library('xmlsec1')#, 'xmlSecInit')
# # barf unless find_header('openssl/ssl.h')
# # barf unless find_header('libxml/tree.h', '/usr/include/libxml2')
# # barf unless find_header('libxslt/xslt.h')
# # barf unless find_header('xmlsec/xmlsec.h', '/usr/include/xmlsec1')

# create_makefile('xmlsecrb')
