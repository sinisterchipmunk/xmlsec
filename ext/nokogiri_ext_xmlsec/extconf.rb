# :stopdoc:

require 'mkmf'

ROOT = File.expand_path(File.join(File.dirname(__FILE__), '..', '..'))

################################################################################
# functions

def lib_a(ldflag)
  case ldflag
  when /\A-l(.+)/
    "lib#{$1}.#{$LIBEXT}"
  end
end

def asplode(lib)
  abort "-----\n#{lib} is missing."
end

def preserving_globals
  values = [
    $arg_config,
    $CFLAGS, $CPPFLAGS,
    $LDFLAGS, $LIBPATH, $libs
  ].map(&:dup)
  yield
ensure
  $arg_config,
  $CFLAGS, $CPPFLAGS,
  $LDFLAGS, $LIBPATH, $libs =
    values
end

def have_iconv?
  have_header('iconv.h') or return false
  %w{ iconv_open libiconv_open }.any? do |method|
    have_func(method, 'iconv.h') or
      have_library('iconv', method, 'iconv.h')
  end
end

def each_iconv_idir
  # If --with-iconv-dir or --with-opt-dir is given, it should be
  # the first priority
  %w[iconv opt].each { |config|
    idir = preserving_globals {
      dir_config(config)
    }.first and yield idir
  }

  # Try the system default
  yield "/usr/include"

  cflags, = preserving_globals {
    pkg_config('libiconv')
  }
  if cflags
    cflags.shellsplit.each { |arg|
      arg.sub!(/\A-I/, '') and
      yield arg
    }
  end

  nil
end

def iconv_prefix
  # Make sure libxml2 is built with iconv
  each_iconv_idir { |idir|
    next unless File.file?(File.join(idir, 'iconv.h'))

    prefix, dir = File.split(idir)
    next unless dir == 'include'

    preserving_globals {
      # Follow the way libxml2's configure uses a value given with
      # --with-iconv[=DIR]
      $CPPFLAGS = "-I#{idir}".quote << ' ' << $CPPFLAGS
      $LIBPATH.unshift(File.join(prefix, "lib"))
      have_iconv?
    } and break prefix
  } or asplode "libiconv"
end

def message!(important_message)
  message important_message
  if !$stdout.tty? && File.chardev?('/dev/tty')
    File.open('/dev/tty', 'w') { |tty|
      tty.print important_message
    }
  end
rescue Errno::ENXIO
end

def process_recipe(name, version, static_p, cross_p)
  MiniPortile.new(name, version).tap do |recipe|
    recipe.target = portsdir = File.join(ROOT, "ports")
    # Prefer host_alias over host in order to use i586-mingw32msvc as
    # correct compiler prefix for cross build, but use host if not set.
    recipe.host = RbConfig::CONFIG["host_alias"].empty? ? RbConfig::CONFIG["host"] : RbConfig::CONFIG["host_alias"]
    recipe.patch_files = Dir[File.join(portsdir, "patches", name, "*.patch")].sort

    yield recipe

    env = Hash.new { |hash, key|
      hash[key] = "#{ENV[key]}"  # (ENV[key].dup rescue '')
    }

    recipe.configure_options.flatten!

    recipe.configure_options.delete_if { |option|
      case option.shellsplit.first
      when /\A(\w+)=(.*)\z/
        env[$1] = $2
        true
      else
        false
      end
    }

    if static_p
      recipe.configure_options += [
        "--disable-shared",
        "--enable-static",
      ]
      env['CFLAGS'] = "-fPIC #{env['CFLAGS']}"
    else
      recipe.configure_options += [
        "--enable-shared",
        "--disable-static",
      ]
    end

    if cross_p
      recipe.configure_options += [
        "--target=#{recipe.host}",
        "--host=#{recipe.host}",
      ]
    end

    if RbConfig::CONFIG['target_cpu'] == 'universal'
      %w[CFLAGS LDFLAGS].each { |key|
        unless env[key].shellsplit.include?('-arch')
          env[key] << ' ' << RbConfig::CONFIG['ARCH_FLAG']
        end
      }
    end

    recipe.configure_options += env.map { |key, value|
      "#{key}=#{value}".shellescape
    }

    if recipe.patch_files.empty?
      message! "Building #{name}-#{version} for nokogiri-xmlsec.\n"
    else
      message! "Building #{name}-#{version} for nokogiri-xmlsec with the following patches applied:\n"

      recipe.patch_files.each { |patch|
        message! "\t- %s\n" % File.basename(patch)
      }
    end

    message! <<-"EOS"
************************************************************************
IMPORTANT! nokogiri-xmlsec builds and uses a packaged version of #{name}.

If this is a concern for you and you want to use the system library
instead, abort this installation process and reinstall nokogiri-xmlsec
as follows:

    gem install nokogiri-xmlsec -- --use-system-libraries

If you are using Bundler, tell it to use the option:

    bundle config build.nokogiri-xmlsec --use-system-libraries
    bundle install
************************************************************************
    EOS

    checkpoint = "#{recipe.target}/#{recipe.name}-#{recipe.version}-#{recipe.host}.installed"
    unless File.exist?(checkpoint)
      recipe.cook
      FileUtils.touch checkpoint
    end
    recipe.activate
  end
end

# Workaround for nokogiri issue #1102
def monkey_patch_mini_portile
  MiniPortile.class_eval do
    def patch
      @patch_files.each do |full_path|
        next unless File.exists?(full_path)
        output "Running patch with #{full_path}..."
        execute('patch', %Q(patch -p1 < #{full_path}))
      end
    end
  end
end

################################################################################
# main

RbConfig::MAKEFILE_CONFIG['CC'] = ENV['CC'] if ENV['CC']

if defined?(RUBY_ENGINE) && RUBY_ENGINE == 'macruby'
  $LIBRUBYARG_STATIC.gsub!(/-static/, '')
end

$CFLAGS << " #{ENV["CFLAGS"]}"
$LIBS << " #{ENV["LIBS"]}"

case RbConfig::CONFIG['target_os']
when 'mingw32', /mswin/
  windows_p = true
  $CFLAGS << " -DXP_WIN -DXP_WIN32 -DUSE_INCLUDED_VASPRINTF"
when /solaris/
  $CFLAGS << " -DUSE_INCLUDED_VASPRINTF"
else
  $CFLAGS << " -g -DXP_UNIX"
end

if RUBY_PLATFORM =~ /mingw/i
  # Work around a character escaping bug in MSYS by passing an arbitrary
  # double quoted parameter to gcc. See https://sourceforge.net/p/mingw/bugs/2142
  $CPPFLAGS << ' "-Idummypath"'
end

if RbConfig::MAKEFILE_CONFIG['CC'] =~ /gcc/
  $CFLAGS << " -O3" unless $CFLAGS[/-O\d/]
  $CFLAGS << " -Wall -Wwrite-strings -Wmissing-noreturn -Winline"
end

case
when arg_config('--use-system-libraries', !!ENV['NOKOGIRI_XMLSEC_USE_SYSTEM_LIBRARIES'])
  message! "Building nokogiri-xmlsec using system libraries.\n"

  dir_config('zlib')

  # Using system libraries means we rely on the system libxml2 with
  # regard to the iconv support.

  dir_config('xml2').any?    or pkg_config('libxml-2.0')
  dir_config('xslt').any?    or pkg_config('libxslt')
  dir_config('exslt').any?   or pkg_config('libexslt')
  dir_config('xmlsec1').any? or pkg_config('xmlsec-openssl')

  try_cpp(<<-SRC) or abort "libxml2 version 2.6.21 or later is required!"
#include <libxml/xmlversion.h>

#if LIBXML_VERSION < 20621
#error libxml2 is too old
#endif
  SRC

  try_cpp(<<-SRC) or warn "libxml2 version 2.9.0 and later is not yet supported, but proceeding anyway."
#include <libxml/xmlversion.h>

#if LIBXML_VERSION >= 20900
#error libxml2 is too new
#endif
  SRC
else
  message! "Building nokogiri-xmlsec using packaged libraries.\n"

  require 'mini_portile'
  monkey_patch_mini_portile
  require 'yaml'

  static_p = enable_config('static', true) or
    message! "Static linking is disabled.\n"

  dependencies = YAML.load_file(File.join(ROOT, "dependencies.yml"))

  cross_build_p = enable_config("cross-build")
  if cross_build_p || windows_p
    zlib_recipe = process_recipe("zlib", dependencies["zlib"], static_p, cross_build_p) do |recipe|
      recipe.files = ["http://zlib.net/#{recipe.name}-#{recipe.version}.tar.gz"]
      class << recipe
        attr_accessor :cross_build_p

        def configure
          Dir.chdir work_path do
            mk = File.read 'win32/Makefile.gcc'
            File.open 'win32/Makefile.gcc', 'wb' do |f|
              f.puts "BINARY_PATH = #{path}/bin"
              f.puts "LIBRARY_PATH = #{path}/lib"
              f.puts "INCLUDE_PATH = #{path}/include"
              mk.sub!(/^PREFIX\s*=\s*$/, "PREFIX = #{host}-") if cross_build_p
              f.puts mk
            end
          end
        end

        def configured?
          Dir.chdir work_path do
            !! (File.read('win32/Makefile.gcc') =~ /^BINARY_PATH/)
          end
        end

        def compile
          execute "compile", "make -f win32/Makefile.gcc"
        end

        def install
          execute "install", "make -f win32/Makefile.gcc install"
        end
      end
      recipe.cross_build_p = cross_build_p
    end

    libiconv_recipe = process_recipe("libiconv", dependencies["libiconv"], static_p, cross_build_p) do |recipe|
      recipe.files = ["http://ftp.gnu.org/pub/gnu/libiconv/#{recipe.name}-#{recipe.version}.tar.gz"]
      recipe.configure_options += [
        "CPPFLAGS='-Wall'",
        "CFLAGS='-O2 -g'",
        "CXXFLAGS='-O2 -g'",
        "LDFLAGS="
      ]
    end
  end

  libxml2_recipe = process_recipe("libxml2", dependencies["libxml2"], static_p, cross_build_p) do |recipe|
    recipe.files = ["ftp://ftp.xmlsoft.org/libxml2/#{recipe.name}-#{recipe.version}.tar.gz"]
    recipe.configure_options += [
      "--without-python",
      "--without-readline",
      "--with-iconv=#{libiconv_recipe ? libiconv_recipe.path : iconv_prefix}",
      "--with-c14n",
      "--with-debug",
      "--with-threads"
    ]
  end

  libxslt_recipe = process_recipe("libxslt", dependencies["libxslt"], static_p, cross_build_p) do |recipe|
    recipe.files = ["ftp://ftp.xmlsoft.org/libxml2/#{recipe.name}-#{recipe.version}.tar.gz"]
    recipe.configure_options += [
      "--without-python",
      "--without-crypto",
      "--with-debug",
      "--with-libxml-prefix=#{libxml2_recipe.path}"
    ]
  end

  xmlsec1_recipe = process_recipe("xmlsec1", dependencies["xmlsec1"], static_p, cross_build_p) do |recipe|
    recipe.files = ["http://www.aleksey.com/xmlsec/download/#{recipe.name}-#{recipe.version}.tar.gz"]
    recipe.configure_options += [
      "--with-libxml=#{libxml2_recipe.path}",
      "--with-libxslt=#{libxslt_recipe.path}",
      "--disable-crypto-dl",
      "--disable-apps-crypto-dl"
    ]
  end

  have_lzma = preserving_globals {
    have_library('lzma')
  }

  $libs = $libs.shellsplit.tap { |libs|
    [libxml2_recipe, libxslt_recipe, xmlsec1_recipe].each { |recipe|
      libname = recipe.name[/\Alib(.+)\z/, 1] || recipe.name
      File.join(recipe.path, "bin", "#{libname}-config").tap { |config|
        # call config scripts explicit with 'sh' for compat with Windows
        $CPPFLAGS = `sh #{config} --cflags`.strip << ' ' << $CPPFLAGS
        `sh #{config} --libs`.strip.shellsplit.each { |arg|
          case arg
          when /\A-L(.+)\z/
            # Prioritize ports' directories
            if $1.start_with?(ROOT + '/')
              $LIBPATH = [$1] | $LIBPATH
            else
              $LIBPATH = $LIBPATH | [$1]
            end
          when /\A-l./
            libs.unshift(arg)
          else
            $LDFLAGS << ' ' << arg.shellescape
          end
        }
      }

      case libname
      when 'xml2'
        # xslt-config --libs or pkg-config libxslt --libs does not include
        # -llzma, so we need to add it manually when linking statically.
        if static_p && have_lzma
          # Add it at the end; GH #988
          libs << '-llzma'
        end
      when 'xslt'
        # xslt-config does not have a flag to emit options including
        # -lexslt, so add it manually.
        libs.unshift('-lexslt')
      end
    }
  }.shelljoin

  if static_p
    $libs = $libs.shellsplit.map { |arg|
      case arg
      when '-lxml2'
        File.join(libxml2_recipe.path, 'lib', lib_a(arg))
      when '-lxslt', '-lexslt'
        File.join(libxslt_recipe.path, 'lib', lib_a(arg))
      else
        arg
      end
    }.shelljoin
  end
    
end

{
  "xml2"  => ['xmlParseDoc',            'libxml/parser.h'],
  "xslt"  => ['xsltParseStylesheetDoc', 'libxslt/xslt.h'],
  "exslt" => ['exsltFuncRegister',      'libexslt/exslt.h']
}.each { |lib, (func, header)|
  have_func(func, header) ||
  have_library(lib, func, header) ||
  have_library("lib#{lib}", func, header) or
    asplode("lib#{lib}")
}

xmlsec1_config = File.join(xmlsec1_recipe.path, "bin", "xmlsec1-config")
xmlsec1_crypto_lib = `sh #{xmlsec1_config} --crypto`.strip

["xmlsec1-#{xmlsec1_crypto_lib}", 'ruby.h'].each { |lib_or_header|
  have_library(lib_or_header) ||
  have_header(lib_or_header) or
    asplode(lib_or_header)
}

create_makefile('nokogiri_ext_xmlsec')

# :startdoc:
