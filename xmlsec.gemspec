# coding: utf-8
lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'xmlsec/version'

Gem::Specification.new do |spec|
  spec.name          = "xmlsec"
  spec.version       = Xmlsec::VERSION
  spec.authors       = ["Colin MacKenzie IV"]
  spec.email         = ["sinisterchipmunk@gmail.com"]
  spec.description   = %q{Wrapper around http://www.aleksey.com/xmlsec to support XML encryption, decryption, signing and signature validation in Ruby}
  spec.summary       = %q{Wrapper around http://www.aleksey.com/xmlsec to support XML encryption, decryption, signing and signature validation in Ruby}
  spec.homepage      = ""
  spec.license       = "MIT"

  spec.files         = `git ls-files`.split($/)
  spec.executables   = spec.files.grep(%r{^bin/}) { |f| File.basename(f) }
  spec.test_files    = spec.files.grep(%r{^(test|spec|features)/})
  spec.require_paths = ["lib"]

  spec.add_dependency 'nokogiri'
  
  spec.add_development_dependency "bundler", "~> 1.3"
  spec.add_development_dependency "rake"
  spec.add_development_dependency "rake-compiler"
  spec.add_development_dependency "rspec"
  spec.add_development_dependency "guard-rspec"
  spec.add_development_dependency "guard-rake"
end
