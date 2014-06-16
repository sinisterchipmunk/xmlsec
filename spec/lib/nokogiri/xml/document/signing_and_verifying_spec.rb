require 'spec_helper'

describe Nokogiri::XML do

  subject { Nokogiri::XML(fixture('sign2-doc.xml')) }

  context 'using an RSA key' do

    describe '.sign!' do
      before { subject.sign! key: fixture('rsa.pem'), name: 'test' }
      it { expect(subject.to_s).to eq(fixture('sign2-result.xml')) }
    end

    describe '.verify_with' do
      before { subject.sign! key: fixture('rsa.pem'), name: 'test' }

      it 'verifies with a single public key' do
        expect(subject.verify_with(key: fixture('rsa.pub'))).to eq(true)
      end

      it 'verifies with a set of keys' do
        expect(subject.verify_with('test' => fixture('rsa.pub'))).to eq(true)
      end
    end
    
  end

  context 'using an RSA key and X509 certificate' do

    describe '.sign!' do
      before do
        subject.sign! key: fixture('cert/server.key.decrypted'),
                      name: 'test',
                      x509: fixture('cert/server.crt')
      end

      it { expect(subject.to_s).to eq fixture('sign3-result.xml') }
    end

    describe '.verify_with' do
      before do
        subject.sign! key: fixture('cert/server.key.decrypted'),
                      name: 'test',
                      x509: fixture('cert/server.crt')
      end

      context 'with an array of certificates' do
        it { expect(subject.verify_with(x509: [fixture('cert/server.crt')])).to eq(true) }
        it { expect(subject.verify_with(certs: [fixture('cert/server.crt')])).to eq(true) }
        it { expect(subject.verify_with(certificates: [fixture('cert/server.crt')])).to eq(true) }

        it 'verifies using system certificates' do
          expect { subject.verify_signature }.not_to raise_error, <<-sys_certs_error
            Could not use system certificates to verify the signature.
            Note that this may not be a failing spec. You should copy
            or symlink the file `spec/fixtures/cert/server.crt` into
            the directory shown by running `openssl version -d`. After
            doing so, run `sudo c_rehash CERT_PATH`, where
            CERT_PATH is the same directory you copied the certificate
            into (/usr/lib/ssl/certs by default on Ubuntu). After doing
            that, run this spec again and see if it passes.
          sys_certs_error
          expect(subject.verify_signature).to eq(true)
        end
      end

      context 'with one certificate' do
        it { expect(subject.verify_with(x509: fixture('cert/server.crt'))).to eq(true) }
        it { expect(subject.verify_with(cert: fixture('cert/server.crt'))).to eq(true) }
        it { expect(subject.verify_with(certificate: fixture('cert/server.crt'))).to eq(true) }
      end
      
    end
    
  end
  
end
