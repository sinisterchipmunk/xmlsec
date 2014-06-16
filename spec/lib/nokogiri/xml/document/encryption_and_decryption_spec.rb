require 'spec_helper'

describe Nokogiri::XML do
  subject { Nokogiri::XML(fixture('sign2-doc.xml')) }
  let!(:original) { subject.to_s }

  context 'using an RSA public key' do
    before { subject.encrypt! key: fixture('rsa.pub'), name: 'test' }

    describe '.encrypt!' do
      it { expect(subject.to_s).not_to eq(original) }
      it { expect(subject.to_s).not_to match(/Hello.*World/i) }
    end

    describe '.decrypt!' do
      before { subject.decrypt! key: fixture('rsa.pem'), name: 'test' }
      it { expect(subject.to_s).to eq(fixture('sign2-doc.xml')) }
    end
    
  end
    
end
