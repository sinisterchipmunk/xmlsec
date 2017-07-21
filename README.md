# nokogiri-xmlsec

Adds support to Ruby for encrypting, decrypting, signing and validating
the signatures of XML documents, according to the [XML Encryption Syntax and
Processing](http://www.w3.org/TR/xmlenc-core/) standard, by wrapping around the
[xmlsec](http://www.aleksey.com/xmlsec) C library and adding relevant methods
to `Nokogiri::XML::Document`.

## Installation

Add this line to your application's Gemfile:

    gem 'nokogiri-xmlsec'

And then execute:

    $ bundle

Or install it yourself as:

    $ gem install nokogiri-xmlsec

## Usage

Several methods are added to `Nokogiri::XML::Document` which expose this gem's
functionality.

### Signing

The `sign!` method adds a digital signature to the XML document so that it can
later be determined whether the document itself has been tampered with. If the
document changes, the signature will be invalid.

Signing a document will add XML nodes directly to the document itself, and
then returns itself.

    # First, get an XML document
    doc = Nokogiri::XML("<doc><greeting>Hello, World!</greeting></doc>")

    # Sign the document with a certificate, a key, and a key name
    doc.sign! certificate: 'certificate data',
              key: 'private key data',
              name: 'private key name'

You only need one of `certificate` or `key`, but you can pass both. If you do,
the certificate will be included as part of the signature, so that it can be
later verified by certificate instead of by key.

The `name` is implicitly converted into a string. Thus it is effectively
optional, since `nil` converts to `""`, and its value only matters if you plan
to verify the signature with any of a set of keys, as in the following example:

### Signature verification

Verification of signatures always returns `true` if successful, `false`
otherwise.

    # Verify the document's signature to ensure it has not been tampered with
    doc.verify_with({
      'key-name-1' => 'public key contents',
      'key-name-2' => 'another public key content'
    })

In the above example, the `name` field from the signing process will be used
to determine which key to validate with. If you plan to always verify with the
same key, you can do it like so, effectively ignoring the `name` value:

    # Verify the document's signature with a specific key
    doc.verify_with key: 'public key contents'

Finally, you can also verify with a certificate:

    # Verify the document's signature with a single certificate
    doc.verify_with certificate: 'certificate data'

    # Verify the document's signature with multiple certificates. Any one match
    # will pass verification.
    doc.verify_with certificates: [ 'cert1', 'cert2', 'cert3' ]

If the certificate has been installed to your system certificates, then you can
verify signatures like so:

    # Verify with installed CA certificates
    doc.verify_signature

### Encryption & Decryption

Encrypted documents can only be decrypted with the private key that corresponds
to the public key that was used to encrypt it. Thus, the party that encrypted
the document can be sure that the document will only be readable by its intended
recipient.

Both encryption and decryption of a document manipulates the XML nodes of the
document in-place. Both methods return the original document, after the changes
have been made to it.

To encrypt a document, use a public key:

    doc.encrypt! key: 'public key content'

To decrypt a document, use a private key:

    doc.decrypt! key: 'private key content'


## Limitations and Known Issues

Following is a list of limitations and/or issues I know about, but have no
immediate plan to resolve. This is probably because I haven't needed the
functionality, and no one has sent a pull request. (Hint, hint!)

- Currently, it is not possible to operate on individual XML nodes. The
  `nokogiri-xmlsec` operations must be performed on the entire document.

## Contributing

1. Fork it
2. Create your feature branch (`git checkout -b my-new-feature`)
3. Commit your changes (`git commit -am 'Add some feature'`)
4. Push to the branch (`git push origin my-new-feature`)
5. Create new Pull Request

gem build nokogiri-xmlsec.gemspec
gem install nokogiri-xmlsec-0.0.5.gem
