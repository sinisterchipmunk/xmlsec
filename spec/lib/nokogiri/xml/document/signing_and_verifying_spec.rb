require 'spec_helper'

describe Nokogiri::XML::Document do
  subject do
    Nokogiri::XML(fixture('sign2-doc.xml'))
  end

  describe 'signing a document with an RSA key' do
    before { subject.sign! key: fixture('rsa.pem'), name: 'test' }

    it 'should produce a signed document' do
      subject.to_s.should == fixture('sign2-result.xml')
    end

    describe 'verifying the document with a single public key' do
      it 'should be valid' do
        subject.verify_with(key: fixture('rsa.pub')).should == true
      end
    end

    describe 'verifying the document with a set of keys' do
      it 'should be valid' do
        subject.verify_with({
          'test' => fixture('rsa.pub')
        }).should == true
      end
    end
  end

  describe 'signing a document with an RSA key and X509 certificate' do
    before do
      subject.sign! key: fixture('cert/server.key.decrypted'),
                    name: 'test',
                    x509: fixture('cert/server.crt')
    end

    it 'should produce a signed document' do
      subject.to_s.should == fixture('sign3-result.xml')
    end

    describe 'verifying the document with an array of X509 certificates' do
      specify { subject.verify_with(x509: [fixture('cert/server.crt')]).should == true }
      specify { subject.verify_with(certs: [fixture('cert/server.crt')]).should == true }
      specify { subject.verify_with(certificates: [fixture('cert/server.crt')]).should == true }

      it 'should verify using system certificates' do
        # subject.verify_signature.should == true -- sort of.
        unless subject.verify_signature
          raise <<-end_error
            Could not use system certificates to verify the signature.
            Note that this may not be a failing spec. You should copy
            or symlink the file `spec/fixtures/cert/server.crt` into
            the directory shown by running `openssl version -d`. After
            doing so, run `sudo c_rehash CERT_PATH`, where
            CERT_PATH is the same directory you copied the certificate
            into (/usr/lib/ssl/certs by default on Ubuntu). After doing
            that, run this spec again and see if it passes.
          end_error
        end
      end
    end

    describe 'verifying the document with one X509 certificate' do
      specify { subject.verify_with(x509: fixture('cert/server.crt')).should == true }
      specify { subject.verify_with(cert: fixture('cert/server.crt')).should == true }
      specify { subject.verify_with(certificate: fixture('cert/server.crt')).should == true }
    end
  end

end
