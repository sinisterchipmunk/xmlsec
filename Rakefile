require "bundler/gem_tasks"
require 'rake/extensiontask'
require 'rspec/core/rake_task'

Rake::ExtensionTask.new('xmlsecrb') do |t|
  t.lib_dir = 'lib/ext'
end

RSpec::Core::RakeTask.new :rspec

desc 'clean out build files'
task :clean do
  rm_rf File.expand_path('../tmp', __FILE__)
end

task :default => [:clean, :compile, :rspec]

desc 'code statistics, cause im a stats junky'
task :stats do
  def count(glob)
    Dir[glob].inject(0) do |count, fi|
      next unless File.file?(fi)
      count + File.read(fi).lines.length
    end
  end

  rb_lines = count 'lib/**/*.rb'
  c_lines  = count 'ext/**/*.{c,h}'

  puts "Lines of Ruby: #{rb_lines}"
  puts "Lines of C:    #{c_lines}"
end
