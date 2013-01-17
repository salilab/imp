"""Utility functions used by all IMP modules"""

import os.path
import re
import sys
from SCons.Script import *
import dependency.compilation
import module
import dependency
import platform
import utility

import SCons

def _get_platform_cxxflags(env):
    ret=[]
    if not env.get('wine', None):
        #from distutils.sysconfig import get_config_vars
        # The compile and link programs used by python must both produce outputs
        # that are compatible with the compiler we are already using as well
        # as much take all command line options we are already using. As a
        # result, we might as well used the same compiler as before. It would
        # be great to check if they match, but that is kind of hard.
        #(oopt, ocflags, oso) = get_config_vars('OPT', 'BASECFLAGS', 'SO')
        opt=utility.get_python_result(env, "import distutils.sysconfig",
                                      "' '.join([x for x in distutils.sysconfig.get_config_vars('OPT')])")
        cflags=utility.get_python_result(env, "import distutils.sysconfig",
                                      "' '.join([x for x in distutils.sysconfig.get_config_vars('BASECFLAGS')])")
        basecflags=[x for x in opt.split()+cflags.split() \
                    if x not in ['-Werror', '-Wall', '-Wextra',
                                 '-O2', '-O3', '-O1', '-Os',
                                 '-fstack-protector', '-Wstrict-prototypes',
                                 '-g', '-dynamic', '-DNDEBUG',
                                 "-fwrapv", "-fno-strict-aliasing"]]
                                            #total.append(v)
                                            # Using _FORTIFY_SOURCE without -O flags triggers a warning on
                                            # newer systems, so remove it
        ret=[x for x in basecflags \
                          if '_FORTIFY_SOURCE' not in x]

    if env.get('cppcoverage', 'no') != 'no':
        if not dependency.gcc.get_is_gcc(env):
            raise ValueError("C coverage testing currently only works with gcc")
        env.Append(CXXFLAGS=["-fprofile-arcs", "-ftest-coverage"])
        if env['build'] == 'debug':
            # gcc info page recommends disabling optimization for optimal
            # coverage reporting
            ret+=["-O0"]
        else:
            print "Warning: It is recommended to build in 'debug' mode " \
                  "when doing C++ coverage testing"

    if dependency.gcc.get_is_gcc(env):
        # "-Werror",  "-Wno-uninitialized"
        ret+=["-Wall", "-Wextra",  "-Wno-deprecated",
              "-Winit-self", "-Wstrict-aliasing=2",
              "-Wcast-align", "-fno-operator-names",
              "-Woverloaded-virtual", "-Wno-unknown-pragmas"]
        if dependency.gcc.get_version(env)>= 4.2:
            if sys.platform == 'darwin':
                ret+=["-Wmissing-prototypes"]
            else:
                ret+=["-Wmissing-declarations"]
        if env['cxx11'] != 'no':
            if dependency.gcc.get_version(env)>= 4.6:
                ret+=["-Wno-c++0x-compat"]
            if dependency.gcc.get_version(env) >= 4.3 and \
                dependency.gcc.get_version(env) < 4.7:
                ret+=["-std=gnu++0x"]
            elif dependency.gcc.get_version(env) >= 4.7:
                ret+=["-std=c++11"]
        #if dependency.gcc.get_version(env)>= 4.3:
        #    env.Append(CXXFLAGS=["-Wunsafe-loop-optimizations"])
        # gcc 4.0 on Mac doesn't like -isystem, so we don't use it there.
        # But without -isystem, -Wundef throws up lots of Boost warnings.
        if sys.platform != 'darwin' or dependency.gcc.get_version(env) > 4.0:
            ret+=["-Wundef"]
        if env['build'] == 'fast':
            ret+=["-O3", "-fexpensive-optimizations",
                                 "-ffast-math", "-ftree-vectorize",
                                 '-ffinite-math-only',
                                 '-fstrict-aliasing',
                                 '-fno-trapping-math',
                                 '-fno-signaling-nans',
                                 '-fno-float-store',
                                 '-funsafe-loop-optimizations',
                                 '--param','inline-unit-growth=200',
                                 '-fearly-inlining',]
            if dependency.gcc.get_version(env)>= 4.3:
                ret+=['-fno-signed-zeros',
                      '-freciprocal-math',
                      '-fassociative-math']
        elif env['build'] == 'release':
            ret+=["-O2", "-g"]
        elif env['build'] == 'compile':
            pass
        elif env['build'] == 'debug':
            ret+=["-g"]
    elif dependency.clang.get_is_clang(env):
        # would be nice, but too much
        #ret+=["-Weverything"]
        # otherwise it whines about our nullptr support
        #ret+=["-Wno-c++98-compat", "-Wno-c++98-compat-pedantic"]
        # otherwise it whines padding in everything
        ret+=["-Wall", "-Wno-unknown-pragmas"]
        ret+=["-Wno-padded"]
        ret+=["-Wno-c++11-extensions"]
        if env['cxx11'] == 'yes':
            ret+=["-std=c++11"]
        if env['build'] == 'fast':
            ret+=["-O3"]
        elif env['build'] == 'release':
            ret+=["-O2", "-g"]
        elif env['build'] == 'compile':
            pass
        elif env['build'] == 'debug':
            # gdb should break on __asan_report_error
            # can't use addresssanitizer at the moment
            # "-faddress-sanitizer"
            ret+=["-g", "-fno-omit-frame-pointer",
                  "-fcatch-undefined-behavior"]
    return ret

def _get_platform_linkflags(env):
    ret=[]
    if env.get('cppcoverage', 'no') != 'no':
        if not dependency.gcc.get_is_gcc(env):
            raise ValueError("C coverage testing currently only works with gcc")
        ret+=["-fprofile-arcs", "-ftest-coverage"]
        if env['build'] == 'debug':
            # gcc info page recommends disabling optimization for optimal
            # coverage reporting
            env.Append(CXXFLAGS=["-O0"])
        else:
            print "Warning: It is recommended to build in 'debug' mode " \
                  "when doing C++ coverage testing"
    if env['PLATFORM'] == 'darwin':
        ret+=['-headerpad_max_install_names']
    if dependency.clang.get_is_clang(env):
        ret+=["-Weverything"]
        if env['build'] == 'debug':
            # gdb should break on __asan_report_error
            #ret+=["-faddress-sanitizer"]
            pass
    return ret


def _update_platform_flags(env):
    if dependency.gcc.get_is_gcc(env):
        env.Replace(IMP_PYTHON_CXXFLAGS=[x for x in env['IMP_PYTHON_CXXFLAGS']
                                     if x not in ['-Wall', '-Wextra', '-Wformat',
                                                  '-Wstrict-aliasing=2',
                                                  '-O3', '-O2',
                                                  "-Wmissing-prototypes",
                                                  "-Wmissing-declarations",
                                         "-Wunused-function",]])
        env.Replace(IMP_BIN_CXXFLAGS=[x for x in env['IMP_BIN_CXXFLAGS']
                                     if x not in ["-Wmissing-prototypes", "-Wmissing-declarations"]])
    elif dependency.clang.get_is_clang(env):
        # just remove warning flags
        env.Replace(IMP_PYTHON_CXXFLAGS=[x for x in env['IMP_PYTHON_CXXFLAGS']
                                     if x not in ["-Weverything",
                                                  "-fcatch-undefined-behavior"]])
        env.Replace(IMP_BIN_CXXFLAGS=[x for x in env['IMP_BIN_CXXFLAGS']
                                     if x not in ["-Wno-missing-prototypes",
                                                  "-Wno-missing-declarations"]])

        # clang notices that python tuples are implemented using the array/struct hack
        env.Append(IMP_PYTHON_CXXFLAGS=["-Wno-array-bounds",
                                        "-Wno-unused-label",
                                        "-Wno-missing-prototypes",
                                        "-Wno-missing-declarations",
                                        "-Wno-unused-function",
                                        "-Wno-self-assign",
                                        "-Wno-unused-value"])
    if env['PLATFORM'] == 'darwin':
        env.Append(IMP_PYTHON_LINKFLAGS=
                ['-flat_namespace', '-undefined', 'suppress'])


def _propagate_variables(env):
    """enforce dependencies between variables"""
    env['IMP_BUILD_STATIC']= env['static']
    env['IMP_BUILD_DYNAMIC']= env['dynamic']
    if env['python'] != "no" and not env.get('PYTHON', None):
        env['IMP_PROVIDE_PYTHON']= True
        if env['python'] =="auto":
            env['PYTHON']=env['python']
        else:
            env['PYTHON']=env['python']
    else:
        env['IMP_PROVIDE_PYTHON']=False
        if env['wine']:
            env['PYTHON']="w32python"
        else:
            env['PYTHON']="python"
    env['IMP_PROVIDE_PYTHON']= env['python']
    env['IMP_USE_PLATFORM_FLAGS']= env['platformflags']
    env['IMP_USE_RPATH']= env['rpath']
    if env['pythonsosuffix'] != 'default':
        env['IMP_PYTHON_SO']=env['pythonsosuffix']
    elif env['IMP_USE_PLATFORM_FLAGS']:
        env['IMP_PYTHON_SO']=utility.get_python_result(env, "import distutils.sysconfig",
                                      "' '.join([x for x in distutils.sysconfig.get_config_vars('SO')])")
    elif env['IMP_PROVIDE_PYTHON']:
        print >> sys.stderr, "Do not know suffix for python lib, please provide pythonsosuffix"
        env.Exit(1)
    if env['wine']:
        env['IMP_BUILD_STATIC']=False
        env['IMP_PYTHON_SO']='.pyd'

    if env['PLATFORM']!= 'posix' and env['PLATFORM'] != 'darwin' and env['IMP_USE_RPATH']:
        env['IMP_USE_RPATH']=False
        print >> sys.stderr, "WARNING rpath not supported on platform "+ env['PLATFORM']

    if not env['IMP_BUILD_DYNAMIC']:
        env['IMP_PROVIDE_PYTHON']=False
    if not env['IMP_BUILD_DYNAMIC'] and not env['IMP_BUILD_STATIC']:
        print >> sys.stderr, "One of dynamic or static libraries must be supported."
        env.Exit(1)
    if env.get('pythonpath', None):
        env['PYTHONPATH'] = env['pythonpath']
    else:
        env['PYTHONPATH']=''
    if env.get('cxxcompiler', None):
        env['CXX']=env['cxxcompiler']
    if env.get('ar', None):
        env['AR']= env['ar']
    if env.get('ranlib', None):
        env['RANLIB']= env['ranlib']
    if env.get("swigprogram", None):
        env['SWIG']= env["swigprogram"]

    # Replace $LINKFLAGS in other link flags so we don't end up
    # adding link flags twice
    env['SHLINKFLAGS'] = env.subst(env['SHLINKFLAGS'])
    env['LDMODULEFLAGS'] = env.subst(env['LDMODULEFLAGS'])
    if isinstance(env['LDMODULEFLAGS'], str):
        env['LDMODULEFLAGS'] = env['LDMODULEFLAGS'].split()

    if env['IMP_USE_PLATFORM_FLAGS']:
        env.Append(CXXFLAGS= _get_platform_cxxflags(env))
        lflags = _get_platform_linkflags(env)
        env.Append(LINKFLAGS=lflags)
        env.Append(SHLINKFLAGS=lflags)
        env.Append(LDMODULEFLAGS=lflags)

    if env.get('cxxflags', None):
        env.Append(CXXFLAGS = env['cxxflags'].split())
    else:
        env.Append(CXXFLAGS=[])
    if env.get('linkflags', None):
        env.Append(LINKFLAGS = env['linkflags'].split())
        env.Append(SHLINKFLAGS = env['linkflags'].split())
        env.Append(LDMODULEFLAGS = env['linkflags'].split())
    else:
        env.Append(LINKFLAGS=[])

    for t in ['includepath', 'libpath', 'datapath','pythonpath', 'swigpath', 'ldlibpath']:
        r=utility.get_abspaths(env, t, env.get(t, ""))
        env[t]=os.path.pathsep.join(r)

    if env.get('includepath') is not None:
        for p in utility.get_env_paths(env, 'includepath'):
            utility.add_to_include_path(env, p)

    if env.get('libpath') is not None:
        env.Prepend(LIBPATH=utility.get_env_paths(env, 'libpath'))
    if env.get('libs') is not None:
        env.Append(LIBS=utility.get_env_paths(env, 'libs'))

    if env.get('ldlibpath') is not None and env.get('ldlibpath') != '':
        dylib_name = utility.get_dylib_name(env)
        if dylib_name:
            env['ENV'][dylib_name] = env['ldlibpath']
    env['ENV']['PATH']= ":".join([env['ENV']['PATH'], Dir("#/build/bin").abspath, env.get('path', "")])
    if env.get('environment') is not None:
        for pair in env.get('environment').split(','):
            if pair != "":
                if pair.find("=") != -1:
                    (name, value)= pair.split("=")
                    env['ENV'][name]=value
                else:
                    env['ENV'][pair]=""

    env['IMP_SHLIB_CXXFLAGS'] = env["CXXFLAGS"]
    env['IMP_ARLIB_CXXFLAGS'] = env["CXXFLAGS"]

    if env.get('pythoncxxflags', None):
        env['IMP_PYTHON_CXXFLAGS'] = env['pythoncxxflags']
    else:
        env['IMP_PYTHON_CXXFLAGS'] = env["CXXFLAGS"]
    if env.get('bincxxflags', None):
        env['IMP_BIN_CXXFLAGS'] = env['bincxxflags']
    else:
        env['IMP_BIN_CXXFLAGS'] = env["CXXFLAGS"]

    if env.get('pythonlinkflags', None):
        env['IMP_PYTHON_LINKFLAGS'] = env['pythonlinkflags']
    else:
        env['IMP_PYTHON_LINKFLAGS'] = env["LDMODULEFLAGS"]

    if env.get('shliblinkflags', None):
        env['IMP_SHLIB_LINKFLAGS'] = env['shliblinkflags']
    else:
        env['IMP_SHLIB_LINKFLAGS'] = env["SHLINKFLAGS"]
    if env.get('arliblinkflags', None):
        env['IMP_ARLIB_LINKFLAGS'] = env['arliblinkflags']
    else:
        env['IMP_ARLIB_LINKFLAGS'] = env["LINKFLAGS"]
    if env.get('binlinkflags', None):
        env['IMP_BIN_LINKFLAGS'] = env['binlinkflags']
    else:
        env['IMP_BIN_LINKFLAGS'] = env["LINKFLAGS"]

    # Make sure variables are lists, not strings
    for key in ('IMP_PYTHON_CXXFLAGS', 'IMP_BIN_CXXFLAGS'):
        if isinstance(env[key], str):
            env[key] = [env[key]]
    if env['IMP_USE_PLATFORM_FLAGS']:
        _update_platform_flags(env)


def add_common_variables(vars, package):
    """Add common variables to an SCons Variables object."""
    libenum=["yes", "no", "auto"]
    libdir = '${prefix}/lib'
    if hasattr(os, 'uname') and sys.platform == 'linux2' \
       and os.uname()[-1] == 'x86_64':
        # Install in /usr/lib64 rather than /usr/lib on x86_64 Linux boxes
        libdir += '64'
    vars.Add(PathVariable('cxxcompiler', 'The C++ compiler to use (eg g++).', None,
                          PathVariable.PathAccept))
    vars.Add(PathVariable('ar', "The command to make a static library.", None,
                          PathVariable.PathAccept))
    vars.Add(PathVariable('ranlib', "The command to make an index of a static library.", None,
                          PathVariable.PathAccept))
    vars.Add(PathVariable('swigprogram', 'The path to the swig command.', None,
                          PathVariable.PathAccept))
    vars.Add(PathVariable('prefix', 'Top-level installation directory', '/usr',
                          PathVariable.PathAccept))
    vars.Add(PathVariable('datadir', 'Data file installation directory',
                          '${prefix}/share/%s'%package,
                          PathVariable.PathAccept))
    vars.Add(PathVariable('bindir', 'Executable installation directory',
                          '${prefix}/bin', PathVariable.PathAccept))
    vars.Add(PathVariable('libdir', 'Shared library installation directory',
                          libdir, PathVariable.PathAccept))
    vars.Add(PathVariable('includedir', 'Include file installation directory',
                          '${prefix}/include', PathVariable.PathAccept))
    vars.Add(PathVariable('pythondir', 'Python module installation directory',
                          '${libdir}/python%d.%d/site-packages' \
                          % sys.version_info[0:2], PathVariable.PathAccept))
    vars.Add(PathVariable('pyextdir',
                          'Python extension module installation directory',
                          '${pythondir}', PathVariable.PathAccept))
    vars.Add(PathVariable('docdir', 'Documentation installation directory',
                          '${prefix}/share/doc/%s' % package,
                          PathVariable.PathAccept))
    vars.Add(PathVariable('cmake', 'The cmake command to use', "cmake",
                          PathVariable.PathAccept))

    # Note that destdir should not affect any compiled-in paths; see
    # http://www.gnu.org/prep/standards/html_node/DESTDIR.html
    vars.Add(PathVariable('destdir',
                          'String to prepend to every installed filename',
                          '', PathVariable.PathAccept))
    vars.Add(PackageVariable('pythoninclude',
                             'Directory holding Python include files ' + \
                             '(if unspecified, distutils location is used)',
                             'no'))
    vars.Add(PackageVariable('modeller',
                             "Set to 'yes' to use the MODELLER package, "
                             "or 'no' to not use it (and to disable modules "
                             "such as IMP.modeller that use it). 'yes' will "
                             "only find MODELLER if it is in the system Python "
                             "path (e.g. Windows, Mac .dmg or Linux .rpm "
                             "binary installs); if you installed the .tar.gz "
                             "version, or have a copy of the source code, set "
                             "this variable to the top-level MODELLER "
                             "directory.", 'no'))
    vars.Add(BoolVariable('wine',
                          'Build using MS Windows tools via Wine emulation',
                          False))
    #vars.Add(BoolVariable('timetests',
    #                      'Print running time for each test',
    #                      False))
    vars.Add(BoolVariable('versionchecks',
                          "By default IMP checks the versions of its dependencies a"
                          "runtime. This is useful to ensure that there are no link "
                          "problems and to avoid very difficult to track down errors. "
                          "In some scenarios, these cannot be implemented properly and "
                          "need to be disabled. If you don't have a good reason, leave "
                          "this on.", True))
    vars.Add(EnumVariable('build',
                          "Set to 'release' for a normal build," \
                          +" 'debug' to disable optimization," \
                          +" or 'fast' to disable most runtime checks," \
                          +" or 'compile' to compile as quickly as possible,"\
                          +" but keep debugging information",
                          "release", ['release', 'debug', 'fast', 'compile']))
    vars.Add(EnumVariable('endian',
                          "The endianness of the platform. \"auto\" will determine it automatically.",
                          "auto", ['auto', 'big', 'little']))
    vars.Add(EnumVariable('nullptr',
                          "Whether the compiler has nullptr. \"auto\" will determine it automatically.",
                          "auto", ['yes', 'no', 'auto']))
    vars.Add(BoolVariable('linksysv',
                          'Link with old-style SysV, not GNU hash, for ' + \
                          'binary compatibility', False))
    vars.Add('includepath', 'Include search path ' + \
             '(e.g. "/usr/local/include:/opt/local/include")', None)
    vars.Add('swigpath', 'Swig search path ' + \
             '(e.g. "/usr/local/share/swig")', None)
    vars.Add('libpath', 'Library search path ' + \
             '(e.g. "/usr/local/lib:/opt/local/lib")', None)
    vars.Add('libs', 'Extra libs to add to link commands ' + \
             '(e.g. "efence:pthread")', None)
    vars.Add(BoolVariable('rpath',
                          'Add any entries from libpath to library search ' + \
                          'path (rpath) on Linux systems', True))
    vars.Add('ldlibpath', 'Add to the runtime library search path ' +\
             '(LD_LIBRARY_PATH on linux-like systems) for various ' + \
             'build tools and the test cases', None)
    vars.Add('cxxflags', 'C++ flags for all C++ builds (e.g. "-fno-rounding:-DFOOBAR"). See pythoncxxflags.',
             None)
    vars.Add('pythoncxxflags', 'C++ flags for building the python libraries (e.g. "-fno-rounding:-DFOOBAR")',
             None)
    vars.Add('bincxxflags', 'C++ flags for building executables libraries (e.g. "-fno-rounding:-DFOOBAR")',
             None)
    vars.Add(EnumVariable('boost_autolink',
                          'Whether to use Boost autolinking to find Boost '
                          'dynamic or static libraries on supported platforms',
                          'disable', ['disable', 'static', 'dynamic'],
                          ignorecase=1))

    vars.Add('linkflags', 'Link flags for all linking (e.g. "-lefence"). See pythonlinkflags, arliblinkflags, shliblinkflags.', None)
    vars.Add(EnumVariable('cxx11',
                          'Whether to use C++ 11 support.',
                          'auto', ['no', 'yes', 'auto']))
    vars.Add('environment', "Add entries to the environment in which tools are run. The variable should be a comma separated list of name=value pairs.", "")
    vars.Add('pythonlinkflags', 'Link flags for linking python libraries (e.g. "-lefence")', "")
    vars.Add('arliblinkflags', 'Link flags for linking static libraries (e.g. "-lefence")', "")
    vars.Add('shliblinkflags', 'Link flags for linking shared libraries (e.g. "-lefence")', "")
    vars.Add('binlinkflags', 'Link flags for linking executables (e.g. "-lefence")', "")
    vars.Add('pkgconfig', 'Whether to use pkg_config ', "auto")
    vars.Add('path', 'Extra executable path ' + \
             '(e.g. "/opt/local/bin/") to search for build tools', None)
    vars.Add('precommand',
             'A command to be run to wrap program invocations.' + \
             'For example, "valgrind --db-attach=yes --suppressions=valgrind-python.supp"', "")
    vars.Add('pythonpath', 'Extra python path ' + \
             '(e.g. "/opt/local/lib/python-2.5/") to use for tests', None)
    vars.Add('boostversion', 'The version of boost. If this is not none, the passed version is used and checks are not done. The version should look like "104200" for Boost "1.42".', None)
    vars.Add('boostlibsuffix', 'The suffix to add onto the boost library names.', 'auto')
    vars.Add(BoolVariable('platformflags',
                          'If true, add any compiler and linker arguments that might be needed/desired. If false, only used passed flags (eg only the values in "cxxflags", "linkflags" etc).',
                          True))
    vars.Add(BoolVariable('deprecated',
                          'Build deprecated classes and functions', True))
    vars.Add('percppcompilation',
                          'By default, all the .cpp files in a module are merged before building, greatly accelerating the process. This can be turned off globally by setting this variable to "yes" or per module by setting it to a colon separated list of module names, eg "em2d:kernel".', "no")
    vars.Add('pythonsosuffix', 'The suffix for the python libraries.', 'default')
    vars.Add('dot',
             'Use dot from graphviz to lay out graphs in the documentation if available. This produces prettier graphs, but is slow.',
                          "auto")
    vars.Add('python', 'The path to python or "no" if python should not be used.', "python")
    vars.Add(BoolVariable('local', 'Whether to build local modules, applications and biological systems that are not part of the IMP distribution', True))
    vars.Add(BoolVariable('linktest', 'Test for header defined functions which are not inline', True))
    vars.Add(PathVariable('repository', 'Where to find the source code to build. This is only needed if building in a different directory than the source.', None, PathVariable.PathAccept)) #PathIsDir
    vars.Add(BoolVariable('static', 'Whether to build static libraries.', False))
    vars.Add(BoolVariable('dynamic', 'Whether to build dynamic libraries (needed for python support).', True))
    vars.Add(BoolVariable('precompiledheader', 'Whether to use a precompiled header for swig libraries ', False))
    vars.Add('disabledmodules', 'A colon-separated list of modules to disable.', '')
    vars.Add('datapath', "The path to the data of an ininstalled IMP you want to use.", None)
    vars.Add(BoolVariable('pretty', "Whether to write cleaner output when building.", True))
    vars.Add(BoolVariable('color', "Whether to write color output output when building.", True))
    vars.Add(EnumVariable('cppcoverage',
                      "Whether to report on C++ code coverage of tests. "
                      '"no" will do no reporting; '
                      '"lines" will list the lines of code that were missed; '
                      '"annotate" will make annotated copies of the code.'
                      " It is strongly recommended to combine this with "
                      "build='debug'. ",
                      'no', ['no', 'lines', 'annotate']))
    vars.Add(EnumVariable('pycoverage',
                      "Whether to report on Python code coverage of tests."
                      '"no" will do no reporting; '
                      '"lines" will list the lines of code that were missed; '
                      '"annotate" will make annotated copies of the code. '
                      'This requires a recent version of the Python coverage '
                      'module installed on your system.',
                      'no', ['no', 'lines', 'annotate']))
    vars.Add(EnumVariable('html_coverage',
                          'Whether to output a coverage report '
                          'in HTML format. (Requires cppcoverage set and the '
                          'lcov package for C output, and pycoverage set '
                          'for Python output.) "single" will output '
                          'a single report that covers all modules or '
                          'applications that were tested with this scons '
                          'invocation; "separate" will generate a separate '
                          'report for each module or application; '
                          'with ":group" some '
                          'modules are grouped together for coverage (e.g. '
                          'much functionality in the kernel is not tested '
                          'there but in the core module.',
                          'no', ['no', 'single', 'separate',
                                 'single:group', 'separate:group']))
    #vars.Add(BoolVariable('noexternaldependencies', 'Do not check files in the provided includepath and libpath for changes.', False))


def update(env, variables):
    variables.Update(env)
    _propagate_variables(env)
