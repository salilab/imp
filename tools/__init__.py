"""Utility functions used by all IMP modules"""

import os.path
import re
import sys
from SCons.Script import *

__all__ = ["add_common_options", "MyEnvironment", "get_pyext_environment",
           "get_sharedlib_environment", "invalidate_environment", "emlib"]

import SCons
_SWIGScanner = SCons.Scanner.ClassicCPP(
    "SWIGScan",
    ".i",
    "CPPPATH",
    '^[ \t]*[%,#][ \t]*(?:include|import)[ \t]*(<|")([^>"]+)(>|")'
)

class WineEnvironment(Environment):
    """Environment to build Windows binaries under Linux, by running the
       MSVC compiler (cl) and linker (link) through wine, using the w32cc
       and w32link shell scripts"""
    def __init__(self, platform='win32', CC='w32cc', LINK='w32link', **kw):
        if sys.platform != 'linux2':
            print "ERROR: Wine is supported only on Linux systems"
            Exit(1)
        Environment.__init__(self, platform=platform, CC=CC, LINK=LINK, **kw)
        posix_env = Environment(platform='posix')
        self['SHLIBPREFIX'] = self['LIBLINKPREFIX'] = 'lib'
        self['WINDOWSEXPPREFIX'] = 'lib'
        self['LIBSUFFIX'] = '.lib'
        self['PSPAWN'] = posix_env['PSPAWN']
        self['SPAWN'] = posix_env['SPAWN']
        self['SHELL'] = posix_env['SHELL']
        self['ENV'] = posix_env['ENV']
        self['PYTHON'] = 'w32python'
        self['PATHSEP'] = ';'
        # Make sure we get the same Windows C/C++ library as Modeller, and
        # enable C++ exception handling
        self.Append(CFLAGS="/MD")
        self.Append(CXXFLAGS="/MD /GR /GX")

def _get_python_include(env):
    """Get the directory containing Python.h"""
    if env['wine']:
        return '/usr/lib/w32comp/w32python/2.5/include/'
    else:
        import distutils.sysconfig
        return distutils.sysconfig.get_python_inc()

def CheckGNUHash(context):
    """Disable GNU_HASH-style linking (if found) for backwards compatibility"""
    context.Message('Checking whether GNU_HASH linking should be disabled...')
    lastLINKFLAGS = context.env['LINKFLAGS']
    context.env.Append(LINKFLAGS="-Wl,--hash-style=sysv")
    text = """
int main(void)
{ return 0; }
"""
    res = context.TryLink(text, '.c')
    if not res:
        context.Result("no")
        context.env.Replace(LINKFLAGS=lastLINKFLAGS)
    else:
        context.Result("yes")
    return res

def CheckGCCVisibility(context):
    """Check if the compiler supports setting visibility of symbols"""
    context.Message('Checking whether compiler supports -fvisibility...')
    lastCCFLAGS = context.env['CCFLAGS']
    context.env.Append(CCFLAGS="-fvisibility=hidden")
    text = """
__attribute__ ((visibility("default")))
int main(void)
{ return 0; }
"""
    res = context.TryLink(text, '.c')
    context.env.Replace(CCFLAGS=lastCCFLAGS)
    if not res:
        context.Result("no")
    else:
        context.env.Append(VIS_CPPDEFINES=['GCC_VISIBILITY'],
                           VIS_CCFLAGS="-fvisibility=hidden")
        context.Result("yes")
    return res

def CheckModeller(context):
    """Find Modeller include and library directories"""
    context.Message('Checking for MODELLER...')
    modeller = context.env['modeller']
    if modeller is False or modeller is 0:
        context.Result("not found")
        return False
    modbin = "%s/bin/modSVN" % modeller
    try:
        (fhin, fhout, fherr) = os.popen3(modbin + " -")
        print >> fhin, "print 'EXE type: ', info.exe_type"
        fhin.close()
    except IOError, e:
        context.Result("could not run MODELLER script %s: %s" % (modbin, e))
        return False
    err = fherr.read()
    exetype = None
    for line in fhout:
        if line.startswith("EXE type"):
            exetype=line[11:].rstrip('\r\n')
    if exetype is None:
        if err:
            context.Result("could not run MODELLER script %s: %s" \
                           % (modbin, err))
        else:
            context.Result("unknown error running MODELLER script %s" % modbin)
        return False
    include = ['%s/src/include' % modeller,
               '%s/src/include/%s' % (modeller, exetype)]
    platform = context.env['PLATFORM']
    if exetype == 'i386-w32':
        libpath = ['%s/src/main' % modeller]
        if platform != 'win32':
            context.Result("MODELLER is built for Windows, but this is not " + \
                           "a Windows scons run (tip: can run on Linux " + \
                           "using Wine with 'scons wine=true'")
            return False
    else:
        libpath = ['%s/lib/%s' % (modeller, exetype)]
        if platform == 'win32':
            context.Result("this is a Windows scons run, but this is not a " + \
                           "Windows MODELLER binary")
            return False
    libs = ["modeller", "saxs"]
    if exetype in ('mac10v4-xlf', 'mac10v4-gnu'):
        libs += ["hdf5", "hdf5_hl"]
    elif exetype == 'mac10v4-intel':
        libs += ["hdf5", "hdf5_hl", "imf", "svml", "ifcore", "irc"]
    modpy = "%s/bin/modpy.sh" % modeller
    # If the modpy.sh script doesn't exist, assume that Modeller will work
    # without it (e.g. on Macs, using the binary .dmg install):
    if not os.path.exists(modpy):
        modpy = ''
    context.env['MODELLER_MODPY'] = modpy
    context.env['MODELLER_EXETYPE'] = exetype
    context.env['MODELLER_CPPPATH'] = include
    context.env['MODELLER_LIBPATH'] = libpath
    context.env['MODELLER_LIBS'] = libs
    context.Result(modeller)
    return True

def _modeller_check_failed(require_modeller):
    """Print an informative message if the Modeller check failed"""
    msg = "  You can set the directory where MODELLER is installed by\n" + \
          "  setting the MODINSTALLSVN environment variable, or with\n" + \
          "  'scons modeller=/opt/modeller' or similar.\n"
    print
    if require_modeller:
        print "ERROR: MODELLER is required to build this package\n\n" + msg
        Exit(1)
    else:
        print "  MODELLER was not found: build will continue but some"
        print "  functionality will be missing.\n\n" + msg


def MyEnvironment(options=None, require_modeller=True, *args, **kw):
    """Create an environment suitable for building IMP modules"""
    import platform
    # First make a dummy environment in order to evaluate all options, since
    # env['wine'] will tell us which 'real' environment to create:
    env = Environment(tools=[], options=options)
    if env['wine']:
        env = WineEnvironment(options=options, *args, **kw)
    else:
        env = Environment(options=options, *args, **kw)
        env['PYTHON'] = 'python'
        env['PATHSEP'] = os.path.pathsep
    try:
        env['SHLINKFLAGS'].remove('-no_archive')
    except ValueError:
        pass
    env.Prepend(SCANNERS = _SWIGScanner)
    if env['CC'] == 'gcc':
        env.Append(CCFLAGS="-Wall -Werror -g -O3")

    sys = platform.system()
    if sys == 'SunOS':
        # Find locally-installed libraries in /usr/local (e.g. for SWIG)
        env['ENV']['LD_LIBRARY_PATH'] = '/usr/local/lib'
    # Make Modeller exetype variable available:
    if os.environ.has_key('EXECUTABLE_TYPESVN'):
        env['ENV']['EXECUTABLE_TYPESVN'] = os.environ['EXECUTABLE_TYPESVN']
    # Set empty variables in case the Modeller check fails:
    for mod in ('MODPY', 'EXETYPE'):
        env['MODELLER_' + mod] = ''
    for mod in ('CPPPATH', 'LIBPATH', 'LIBS'):
        env['MODELLER_' + mod] = []
    # Note: would like to check for 'help' here too, but that requires a
    # post 0.97 scons snapshot
    if not env.GetOption('clean'):
        custom_tests = {'CheckGNUHash': CheckGNUHash,
                        'CheckGCCVisibility': CheckGCCVisibility,
                        'CheckModeller': CheckModeller}
        conf = env.Configure(custom_tests = custom_tests)
        if sys == 'Linux':
            conf.CheckGNUHash()
        conf.CheckGCCVisibility()
        # Check explicitly for False, since all checks will return Null if
        # configure has been disabled
        if conf.CheckModeller() is False:
            _modeller_check_failed(require_modeller)
        conf.Finish()
    return env

def _fix_aix_cpp_link(env, cplusplus, linkflags):
    """On AIX things get confused if AIX C but not AIX C++ is installed - AIX C
       options get passed to g++ - so hard code GNU link flags"""
    if cplusplus and 'aixcc' in env['TOOLS'] and 'aixc++' not in env['TOOLS'] \
       and 'g++' in env['TOOLS']:
        slflags = str(env[linkflags])
        env[linkflags] = slflags.replace('-qmkshrobj -qsuppress=1501-218',
                                         '-shared')

def get_sharedlib_environment(env, cppdefine, cplusplus=False):
    """Get a modified environment suitable for building shared libraries
       (i.e. using gcc ELF visibility macros or MSVC dllexport/dllimport macros
       to mark dynamic symbols as exported or private). `cppdefine` should be
       the name of a cpp symbol to define to tell MSVC that we are building the
       library (by convention something of the form FOO_EXPORTS).
       If `cplusplus` is True, additional configuration suitable for a C++
       shared library is done."""
    e = env.Clone()
    e.Append(CPPDEFINES=[cppdefine, '${VIS_CPPDEFINES}'],
             CCFLAGS='${VIS_CCFLAGS}')

    _fix_aix_cpp_link(e, cplusplus, 'SHLINKFLAGS')
    return e

def get_pyext_environment(env, cplusplus=False):
    """Get a modified environment for building a Python extension.
       If `cplusplus` is True, additional configuration suitable for a C++
       extension is done."""
    from platform import system
    e = env.Clone()
    if 'swig' not in e['TOOLS'] and not env.GetOption('clean'):
        print "ERROR: SWIG could not be found. SWIG is needed to build."
        Exit(1)
    e['LDMODULEPREFIX'] = ''
    # We're not going to link against the extension, so don't need a Windows
    # import library (.lib file):
    e['no_import_lib'] = 1
    platform = e['PLATFORM']
    if e['wine']:
        # Have to set SHLIBSUFFIX and PREFIX on Windows otherwise the
        # mslink tool complains
        e['SHLIBPREFIX'] = ''
        e['LDMODULESUFFIX'] = e['SHLIBSUFFIX'] = '.pyd'
        # Directory containing python25.lib:
        e.Append(LIBPATH=['/usr/lib/w32comp/w32python/2.5/lib/'])
    else:
        if platform == 'aix':
            # Make sure compilers are in the PATH, so that Python's script for
            # building AIX extension modules can find them:
            e['ENV']['PATH'] += ':/usr/vac/bin'
        from distutils.sysconfig import get_config_vars
        vars = get_config_vars('CC', 'CXX', 'OPT', 'BASECFLAGS', 'LDSHARED',
                               'SO')
        (cc, cxx, opt, basecflags, ldshared, so) = vars
        # distutils on AIX can get confused if AIX C but GNU C++ is installed:
        if platform == 'aix' and cxx == '':
            cxx = 'g++'
        # Don't require stack protector stuff on Linux, as this adds a
        # requirement for glibc-2.4:
        opt = opt.replace("-fstack-protector", "")
        # Remove options that don't work with C++ code:
        if cplusplus:
            opt = opt.replace("-Wstrict-prototypes", "")
        e.Replace(CC=cc, CXX=cxx, LDMODULESUFFIX=so)
        e.Replace(CPPFLAGS=basecflags.split() + opt.split())
        # Some gcc versions don't like the code that SWIG generates - but let
        # that go, because we have no control over it:
        try:
            e['CCFLAGS'].remove('-Werror')
        except ValueError:
            pass
        # AIX tries to use the C compiler rather than g++, so hardcode it here:
        if platform == 'aix' and cplusplus:
            ldshared = ldshared.replace(' cc_r', ' g++')
        # Default link flags on OS X don't work for us:
        if platform == 'darwin':
            e.Replace(LDMODULEFLAGS= \
                      '$LINKFLAGS -bundle -flat_namespace -undefined suppress')
        # Don't set link flags on Linux, as that would negate our GNU_HASH check
        elif system() != "Linux":
            e['LDMODULEFLAGS'] = []
            e['SHLINK'] = e['LDMODULE'] = ldshared
    e.Append(CPPPATH=[_get_python_include(e)])
    _fix_aix_cpp_link(e, cplusplus, 'SHLINKFLAGS')
    return e

def invalidate_environment(env, fail_builder):
    """'Break' an environment, so that any builds with it use the fail_builder
       function (which should be an Action which terminates the build)"""
    for var in ('SHLINKCOM', 'CCCOM', 'CXXCOM', 'SHCCCOM', 'SHCXXCOM'):
        env[var] = fail_builder

def add_common_options(opts, package):
    """Add common options to an SCons Options object."""
    opts.Add(PathOption('prefix', 'Top-level installation directory', '/usr',
                        PathOption.PathAccept))
    opts.Add(PathOption('datadir', 'Data file installation directory',
                        '${prefix}/share/%s' % package, PathOption.PathAccept))
    opts.Add(PathOption('libdir', 'Shared library installation directory',
                        '${prefix}/lib', PathOption.PathAccept))
    opts.Add(PathOption('includedir', 'Include file installation directory',
                        '${prefix}/include', PathOption.PathAccept))
    opts.Add(PathOption('pythondir', 'Python module installation directory',
                        '${prefix}/lib/python%d.%d/site-packages' \
                        % sys.version_info[0:2], PathOption.PathAccept))
    opts.Add(PathOption('pyextdir',
                        'Python extension module installation directory',
                        '${pythondir}', PathOption.PathAccept))
    opts.Add(PathOption('docdir', 'Documentation installation directory',
                        '${prefix}/share/doc/%s' % package,
                        PathOption.PathAccept))
    opts.Add(PackageOption('modeller', 'Location of the MODELLER package',
                           os.environ.get('MODINSTALLSVN', False)))
    opts.Add(BoolOption('wine',
                        'Build using MS Windows tools via Wine emulation',
                        False))
