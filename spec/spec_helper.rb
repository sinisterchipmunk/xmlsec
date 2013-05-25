require 'rspec'
require 'xmlsec'

def fixture_path(filename)
  File.join(File.expand_path('../fixtures', __FILE__), filename)
end

def fixture(path)
  File.read fixture_path(path)
end
