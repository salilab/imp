"""Utility functions used by all IMP modules"""

import os.path
import re
import sys
import popen2
from SCons.Script import *
import hierarchy
import generate_doxygen
import make_examples
import symlinks

__all__ = ["add_common_variables", "MyEnvironment", "get_pyext_environment",
           "get_sharedlib_environment", "embed"]

import SCons
_SWIGScanner = SCons.Scanner.ClassicCPP(
    "SWIGScan",
    ".i",
    "CPPPATH",
    '^[ \t]*[%,#][ \t]*(?:include|import)[ \t]*(<|")([^>"]+)(>|")'
)

# Provide a portable way to use the popen2.Popen3 class (we need this rather
# than the popen3() factory function so we can call wait() to get exit status,
# and can't use subprocess just yet since that requires a newer Python).
if hasattr(popen2, 'Popen3'):
    MyPopen3 = popen2.Popen3
else:
    class MyPopen3(object):
        def __init__(self, cmd, capturestderr):
            (self.tochild, self.fromchild, self.childerr) \
                = os.popen3(cmd, 't', -1)
        def wait(self):
            return 0

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
        # Use / rather than \ path separator:
        self['LINKCOM'] = self['LINKCOM'].replace('.windows', '')
        # Make sure we get the same Windows C/C++ library as Modeller, and
        # enable C++ exception handling
        self.Append(CFLAGS="/MD")
        self.Append(CXXFLAGS="/MD /GR /GX")

def _get_python_include(env):
    """Get the directory containing Python.h"""
    if env['python_include']:
        return env['python_include']
    elif env['wine']:
        return '/usr/lib/w32comp/w32python/2.6/include/'
    else:
        import distutils.sysconfig
        return distutils.sysconfig.get_python_inc()

def _add_build_flags(env):
    """Add compiler flags for release builds, if requested"""
    #make sure they are all there
    env.Append(CPPPATH=[])
    env.Append(CCFLAGS=[])
    env.Append(LINKFLAGS=[])
    env.Append(LIBPATH=[])
    if env['CC'] == 'gcc':
        env.Append(CCFLAGS=["-Wall"])
    if env.get('build', 'release') == 'fast':
        env.Append(CPPDEFINES=['NDEBUG'])
        if env['CC'] == 'gcc':
            env.Append(CCFLAGS=[ "-O3"])
    elif env.get('build', 'release') == 'release':
        if env['CC'] == 'gcc':
            env.Append(CCFLAGS=["-O2"])
    elif env.get('build', 'release') == 'debug':
        if env['CC'] == 'gcc':
            env.Append(CCFLAGS=["-g"])
    elif env.get('build', 'release') == 'profile':
        env.Append(CPPDEFINES=['NDEBUG'])
        if env['CC'] == 'gcc':
            env.Append(CCFLAGS=[ "-O3"])
            env.Append(CCFLAGS=['-g', '-pg'])
            env.Append(LINKFLAGS=['-pg'])
    else:
        print "ERROR: Invalid build mode, should be one of debug, release, fast, profile."
        Exit(1)

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
    # Find MODELLER script
    moddir = "%s/bin" % modeller
    try:
        files = os.listdir(moddir)
    except OSError, e:
        context.Result("could not find MODELLER directory %s: %s" % (moddir, e))
        return False
    files.sort()
    r = re.compile('mod(SVN|\d+v\d+)$')
    files = [f for f in files if r.match(f)]
    if len(files) == 0:
        context.Result("could not find MODELLER script in %s" % moddir)
        return False
    # Last matching entry is probably the latest version:
    modbin = os.path.join(moddir, files[-1])
    try:
        p = MyPopen3(modbin + " -", True)
        print >> p.tochild, "print 'EXE type: ', info.exe_type"
        p.tochild.close()
    except IOError, e:
        context.Result("could not run MODELLER script %s: %s" % (modbin, e))
        return False
    err = p.childerr.read()
    exetype = None
    for line in p.fromchild:
        if line.startswith("EXE type"):
            exetype=line[11:].rstrip('\r\n')
    ret = p.wait()
    if exetype is None:
        if err or ret != 0:
            context.Result("could not run MODELLER script %s: %d, %s" \
                           % (modbin, ret, err))
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
    msg = "  Use the modeller command line option (or options file) to\n" + \
          "  set the directory where Modeller is installed\n" + \
          "  (run 'scons -h' for help.)"

    print
    if require_modeller:
        print "ERROR: MODELLER is required to build this package\n\n" + msg
        Exit(1)
    else:
        print "  MODELLER was not found: build will continue but some"
        print "  functionality will be missing.\n\n" + msg


def MyEnvironment(variables=None, require_modeller=True, *args, **kw):
    """Create an environment suitable for building IMP modules"""
    import platform
    # First make a dummy environment in order to evaluate all variables, since
    # env['wine'] will tell us which 'real' environment to create:
    env = Environment(tools=[], variables=variables)
    newpath = env['ENV']['PATH']
    if env.get('path', None) is not None:
        newpath += os.path.pathsep + env['path']
    if env['wine']:
        env = WineEnvironment(variables=variables, ENV = {'PATH': newpath},
                              *args, **kw)
    else:
        env = Environment(variables=variables, ENV = {'PATH': newpath},
                          *args, **kw)
        env['PYTHON'] = 'python'
        env['PATHSEP'] = os.path.pathsep
    try:
        env['SHLINKFLAGS'] = str(env['SHLINKFLAGS']).replace('-no_archive', '')
    except ValueError:
        pass
    env['PYTHONPATH'] = '#/build/lib'
    env.AddMethod(symlinks.LinkInstall)
    env.AddMethod(symlinks.LinkInstallAs)
    env.AddMethod(hierarchy.InstallHierarchy)
    env.Prepend(SCANNERS = [_SWIGScanner],
                BUILDERS = {'GenerateDoxFromIn':
                            generate_doxygen.GenerateDoxFromIn,
                            'MakeExamples': make_examples.MakeExamples})
    if env.get('cxxflags', None) is not None:
        env.Append(CXXFLAGS = [env['cxxflags'].split(" ")])
    if env.get('linkflags', None) is not None:
        env.Append(LINKFLAGS=[env['linkflags'].split(" ")])

    if env.get('include', None) is not None:
        env['include'] = [os.path.abspath(x) for x in \
                          env['include'].split(os.path.pathsep)]
        env.Append(CPPPATH=env['include'])
    # make sure it is there
    env.Append(LIBPATH=[])
    if env.get('lib', None) is not None:
        env['lib'] = [os.path.abspath(x) for x in \
                      env['lib'].split(os.path.pathsep)]
        env.Append(LIBPATH=env['lib'])
    else:
        env['lib'] = []
    _add_build_flags(env)

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
    if not env.GetOption('clean') and not env.GetOption('help'):
        custom_tests = {'CheckGNUHash': CheckGNUHash,
                        'CheckGCCVisibility': CheckGCCVisibility,
                        'CheckModeller': CheckModeller}
        conf = env.Configure(custom_tests = custom_tests)
        if sys == 'Linux' and env['linksysv']:
            conf.CheckGNUHash()
        if sys != 'win32' and not env['wine']:
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
    if e['PLATFORM'] == 'posix' or e['PLATFORM']=='sunos':
        dylinkflags=[]
        for p in e['LIBPATH']:
            if p[0] is not '#':
                # append/prepend must match other uses
                dylinkflags.append('-Wl,-rpath,'+p)
        e.Prepend(LINKFLAGS=dylinkflags)
    _fix_aix_cpp_link(e, cplusplus, 'SHLINKFLAGS')
    return e

# 1. Workaround for SWIG bug #1863647: Ensure that the PySwigIterator class is
#    renamed with a module-specific prefix, to avoid collisions when using
#    multiple modules
# 2. If module names contain '.' characters, SWIG emits these into the CPP
#    macros used in the director header. Work around this by replacing them
#    with '_'. A longer term fix is not to call our modules "IMP.foo" but
#    to say %module(package=IMP) foo but this doesn't work in SWIG stable
#    as of 1.3.36 (Python imports incorrectly come out as 'import foo'
#    rather than 'import IMP.foo'). See also IMP bug #41 at
#    https://salilab.org/imp/bugs/show_bug.cgi?id=41
class _swig_postprocess(object):
    def __init__(self, modprefix):
        self.modprefix = modprefix
    def builder(self, source, target, env):
        for t in target:
            path = t.path
            if path.endswith('.cc'):
                lines = file(path, 'r').readlines()
                repl = '"swig::IMP%s_PySwigIterator *"' % self.modprefix
                fh = file(path, 'w')
                for line in lines:
                    fh.write(line.replace('"swig::PySwigIterator *"', repl))
                fh.close()
            elif path.endswith('.h'):
                lines = file(path, 'r').readlines()
                orig = 'SWIG_IMP.%s_WRAP_H_' % self.modprefix.lower()
                repl = 'SWIG_IMP_%s_WRAP_H_' % self.modprefix
                fh = file(path, 'w')
                for line in lines:
                    fh.write(line.replace(orig, repl))
                fh.close()
        return 0

def get_pyext_environment(env, mod_prefix, cplusplus=False):
    """Get a modified environment for building a Python extension.
       `mod_prefix` should be a unique prefix for this module.
       If `cplusplus` is True, additional configuration suitable for a C++
       extension is done."""
    from platform import system
    e = env.Clone()
    if 'swig' not in e['TOOLS'] and not env.GetOption('clean'):
        print "ERROR: SWIG could not be found. SWIG is needed to build."
        Exit(1)

    if cplusplus and isinstance(e['SWIGCOM'], str):
        # See _swig_postprocess class comments:
        repl = '$SWIG -DPySwigIterator=IMP%s_PySwigIterator ' % mod_prefix
        e['SWIGCOM'] = e['SWIGCOM'].replace('$SWIG ', repl)
        e['SWIGCOM'] = [e['SWIGCOM'], _swig_postprocess(mod_prefix).builder]

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
        # Directory containing python26.lib:
        e.Append(LIBPATH=['/usr/lib/w32comp/w32python/2.6/lib/'])
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
        if e['PLATFORM'] is 'posix':
            for p in e['LIBPATH']:
                if p[0] is not '#':
                    # append/prepend must match other uses
                    e.Prepend(LINKFLAGS=['-Wl,-rpath,'+p])
        # Remove NDEBUG preprocessor stuff if defined (we do it ourselves for
        # release builds)
        if '-DNDEBUG' in e['CPPFLAGS']:
            e['CPPFLAGS'].remove('-DNDEBUG')
        if '-Wall' in e['CCFLAGS']:
            e['CCFLAGS'].remove('-Wall')

        # Some gcc versions don't like the code that SWIG generates - but let
        # that go, because we have no control over it:
        for (var, f) in (('CCFLAGS', '-Werror'), ('CPPFLAGS', '-Wall'),
                         ('CCFLAGS', '-Wall')):
            try:
                e[var].remove(f)
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

def add_common_variables(vars, package):
    """Add common variables to an SCons Variables object."""
    vars.Add(PathVariable('prefix', 'Top-level installation directory', '/usr',
                          PathVariable.PathAccept))
    vars.Add(PathVariable('datadir', 'Data file installation directory',
                          '${prefix}/share/%s' % package,
                          PathVariable.PathAccept))
    vars.Add(PathVariable('libdir', 'Shared library installation directory',
                          '${prefix}/lib', PathVariable.PathAccept))
    vars.Add(PathVariable('includedir', 'Include file installation directory',
                          '${prefix}/include', PathVariable.PathAccept))
    vars.Add(PathVariable('pythondir', 'Python module installation directory',
                          '${prefix}/lib/python%d.%d/site-packages' \
                          % sys.version_info[0:2], PathVariable.PathAccept))
    vars.Add(PathVariable('pyextdir',
                          'Python extension module installation directory',
                          '${pythondir}', PathVariable.PathAccept))
    vars.Add(PathVariable('docdir', 'Documentation installation directory',
                          '${prefix}/share/doc/%s' % package,
                          PathVariable.PathAccept))
    vars.Add(PackageVariable('python_include',
                             'Directory holding Python include files ' + \
                             '(if unspecified, distutils location is used)',
                             'no'))
    vars.Add(PackageVariable('modeller', 'Location of the MODELLER package',
                             'no'))
    vars.Add(BoolVariable('wine',
                          'Build using MS Windows tools via Wine emulation',
                          False))
    vars.Add(PathVariable('build',
                          "Set to \'release\' for a normal build,"
                          +" or debug to disable optimization,"
                          +" or fast to disable most runtime checks"
                          +" or profile to disable most runtime checks"
                          +"but keep debugging information",
                          "release", PathVariable.PathAccept))
    vars.Add(BoolVariable('linksysv',
                          'Link with old-style SysV, not GNU hash, for ' + \
                          'binary compatibility', False))
    vars.Add(PathVariable('include', 'Include search path ' + \
                          '(e.g. "/usr/local/include:/opt/local/include")',
                          None, PathVariable.PathAccept))
    vars.Add(PathVariable('lib', 'Library search path ' + \
                          '(e.g. "/usr/local/lib:/opt/local/lib")', None,
                          PathVariable.PathAccept))
    vars.Add(PathVariable('cxxflags', 'Extra cxx flags ' + \
                          '(e.g. "-fno-rounding -DFOOBAR")',
                          None, PathVariable.PathAccept))
    vars.Add(PathVariable('linkflags', 'Extra link flags ' + \
                          '(e.g. "-lefence")', None,
                          PathVariable.PathAccept))
    vars.Add(PathVariable('path', 'Extra executable path ' + \
                          '(e.g. "/opt/local/bin/")', None,
                          PathVariable.PathAccept))
    vars.Add(PathVariable('pythonpath', 'Extra python path ' + \
                          '(e.g. "/opt/local/lib/python-2.5/")', None,
                          PathVariable.PathAccept))
