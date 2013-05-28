require "xmlsec/version"
require 'nokogiri'
require 'nokogiri_ext_xmlsec'

class Nokogiri::XML::Document
  # Signs this document, and then returns it.
  #
  # Examples:
  #
  #     doc.sign! key: 'rsa-private-key'
  #     doc.sign! key: 'rsa-private-key', name: 'key-name'
  #     doc.sign! x509: 'x509 certificate', key: 'cert private key'
  #     doc.sign! x509: 'x509 certificate', key: 'cert private key',
  #               name: 'key-name'
  #
  # You can also use `:cert` or `:certificate` as aliases for `:x509`.
  #
  def sign! opts
    if (cert = opts[:x509]) || (cert = opts[:cert]) || (cert = opts[:certificate])
      raise "need a private :key" unless opts[:key]
      sign_with_certificate opts[:name].to_s, opts[:key], cert
    elsif opts[:key]
      sign_with_key opts[:name].to_s, opts[:key]
    else
      raise "No private :key was given"
    end
    self
  end

  # Verifies the signature on the current document.
  #
  # Returns `true` if the signature is valid, `false` otherwise.
  # 
  # Examples:
  #
  #     # Try to validate with the given public or private key
  #     doc.verify_with key: 'rsa-key'
  #
  #     # Try to validate with a set of keys. It will try to match
  #     # based on the contents of the `KeyName` element.
  #     doc.verify_with({
  #       'key-name'         => 'x509 certificate',
  #       'another-key-name' => 'rsa-public-key'
  #     })
  #     
  #     # Try to validate with a trusted certificate
  #     doc.verify_with(x509: 'certificate')
  #
  #     # Try to validate with a set of certificates, any one of which
  #     # can match
  #     doc.verify_with(x509: ['cert1', 'cert2'])
  #
  # You can also use `:cert` or `:certificate` or `:certs` or
  # `:certificates` as aliases for `:x509`.
  #
  def verify_with opts_or_keys
    if (certs = opts_or_keys[:x509]) ||
       (certs = opts_or_keys[:cert]) ||
       (certs = opts_or_keys[:certs]) ||
       (certs = opts_or_keys[:certificate]) ||
       (certs = opts_or_keys[:certificates])
      certs = [certs] unless certs.kind_of?(Array)
      verify_with_certificates certs
    elsif opts_or_keys[:key]
      verify_with_rsa_key opts_or_keys[:key]
    else
      verify_with_named_keys opts_or_keys
    end
  end

  # Attempts to verify the signature of this document using only certificates
  # installed on the system. This is equivalent to calling
  # `verify_with certificates: []` (that is, an empty array).
  #
  def verify_signature
    verify_with_certificates []
  end

  # Encrypts the current document, then returns it.
  #
  # Examples:
  # 
  #     # encrypt with a public key and optional key name
  #     doc.encrypt! key: 'public-key', name: 'name'
  #
  def encrypt! opts
    if opts[:key]
      encrypt_with_key opts[:name].to_s, opts[:key]
    else
      raise "private :key is required for encryption"
    end
    self
  end

  # Decrypts the current document, then returns it.
  #
  # Examples:
  #
  #     # decrypt with a specific private key
  #     doc.decrypt! key: 'private-key'
  #
  def decrypt! opts
    if opts[:key]
      decrypt_with_key opts[:name].to_s, opts[:key]
    else
      raise 'inadequate options specified for decryption'
    end
    self
  end
end
