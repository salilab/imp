"""Utility functions used by all IMP modules"""

import os.path
import re
import sys
from SCons.Script import *
import hierarchy
import symlinks
import standards
import compilation
import swig

__all__ = ["add_common_variables", "MyEnvironment", "get_pyext_environment",
           "get_sharedlib_environment"]

import SCons


def GetInstallDirectory(env, varname, *subdirs):
    """Get a directory to install files in. The top directory is env[varname],
       prefixed with env['destdir']. The full directory is constructed by
       adding any other function arguments as subdirectories."""
    destdir = env.subst(env['destdir'])
    installdir = env.subst(env[varname])
    if destdir != '' and not os.path.isabs(installdir):
        print "Install directory %s (%s) must be an absolute path,\n" \
              "since you have set destdir." % (varname, installdir)
        env.Exit(1)
    installdir = destdir + installdir
    # Use SCons, not os.path.abspath, since we may not be in the top directory
    if not os.path.isabs(installdir):
        installdir = env.Dir('#/' + installdir).abspath
    return os.path.join(installdir, *subdirs)

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
    if env['CXX'] == 'g++':
        env.Append(CXXFLAGS=["-Woverloaded-virtual"])
        env['use_pch']=env['precompiledheader']
    else:
        env['use_pch']=False
    if env['build'] == 'fast':
        if env['CC'] == 'gcc':
            env.Append(CCFLAGS=["-O3", "-fexpensive-optimizations",
                                "-ffast-math"])
            env.Append(CPPDEFINES=["NDEBUG"])
    elif env['build'] == 'release':
        if env['CC'] == 'gcc':
            env.Append(CCFLAGS=["-O2"])
    elif env['build'] == 'debug':
        if env['CC'] == 'gcc':
            env.Append(CCFLAGS=["-g"])


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


def MyEnvironment(variables=None, *args, **kw):
    """Create an environment suitable for building IMP modules"""
    import platform
    #import colorizer
    # First make a dummy environment in order to evaluate all variables, since
    # env['wine'] will tell us which 'real' environment to create:
    env = Environment(tools=[], variables=variables)
    newpath = env['ENV']['PATH']
    if env.get('path') is not None:
        newpath = env['path'] + os.path.pathsep + newpath
    if env['wine']:
        env = WineEnvironment(variables=variables, ENV = {'PATH': newpath},
                              *args, **kw)
    else:
        env = Environment(variables=variables, ENV = {'PATH': newpath},
                          *args, **kw)
        env['PYTHON'] = 'python'
        env['PATHSEP'] = os.path.pathsep
    try:
        env['SHLINKFLAGS'] = [ x.replace('-no_archive', '') for x in env['SHLINKFLAGS']]
    except ValueError:
        pass
    #col = colorizer.colorizer()
    #col.colorize(env)
    env['PYTHONPATH'] = '#/build/lib'
    env['all_modules']=[]
    if env['rpath']:
        if env['PLATFORM']!= 'posix' and env['PLATFORM'] != 'darwin':
            env['rpath']=False
            print "WARNING rpath not supported on platform "+ env['PLATFORM']
    #env.Decider('MD5-timestamp')
    env.AddMethod(symlinks.LinkInstall)
    env.AddMethod(symlinks.LinkInstallAs)
    env.AddMethod(hierarchy.InstallHierarchy)
    env.AddMethod(GetInstallDirectory)
    if env.get('cxxflags'):
        env.Append(CXXFLAGS = env['cxxflags'].split(" "))
    if env.get('linkflags'):
        env.Append(LINKFLAGS=[env['linkflags'].split(" ")])

    if env.get('includepath') is not None:
        env['includepath'] = [os.path.abspath(x) for x in \
                          env['includepath'].split(os.path.pathsep)]
        env.Prepend(CPPPATH=env['includepath'])
    # make sure it is there
    env.Append(LIBPATH=[])
    if env.get('libpath') is not None:
        env['libpath'] = [os.path.abspath(x) for x in \
                      env['libpath'].split(os.path.pathsep)]
        env.Prepend(LIBPATH=env['libpath'])
    else:
        env['libpath'] = []
    if env.get('libs') is not None:
        libs= env['libs'].split(":")
        env.Append(LIBS=libs);
    _add_build_flags(env)

    sys = platform.system()
    if env.get('ldlibpath') is not None:
        env['ENV']['LD_LIBRARY_PATH'] = env['ldlibpath']
    # Make Modeller exetype variable available:
    if os.environ.has_key('EXECUTABLE_TYPESVN'):
        env['ENV']['EXECUTABLE_TYPESVN'] = os.environ['EXECUTABLE_TYPESVN']
    # Set empty variables in case the Modeller check fails:
    for mod in ('MODPY', 'EXETYPE'):
        env['MODELLER_' + mod] = ''
    for mod in ('CPPPATH', 'LIBPATH', 'LIBS'):
        env['MODELLER_' + mod] = []
    env['SVNVERSION'] = env.WhereIs('svnversion')
    if env['svn'] and not env['SVNVERSION']:
        print "Warning: Could not find 'svnversion' binary in path"
    #if not env.GetOption('clean') and not env.GetOption('help'):
    if True:
        compilation.configure_check(env)
        custom_tests = {'CheckGNUHash': CheckGNUHash,
                        'CheckGCCVisibility': CheckGCCVisibility}
        conf = env.Configure(custom_tests = custom_tests)
        if sys == 'Linux' and env['linksysv']:
            conf.CheckGNUHash()
        if sys != 'win32' and not env['wine']:
            conf.CheckGCCVisibility()
        # Check explicitly for False, since all checks will return Null if
        # configure has been disabled
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

def _add_rpath(env):
    # when supported, change the rpath so that libraries can be found at runtime
    # also, add to the rpath used when linking so the linux linker can resolve
    # inter-library dependencies
    if env['PLATFORM'] == 'posix':
        dylinkflags=[]
        for p in env['LIBPATH']:
            if p[0] is not '#':
                # append/prepend must match other uses
                if  env['rpath']:
                    env.Prepend(LINKFLAGS=['-Wl,-rpath,'+p])
                env.Prepend(LINKFLAGS=['-Wl,-rpath-link,'+p])
        env.Prepend(LINKFLAGS=['-Wl,-rpath-link,'+Dir("#/build/lib").abspath])

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
    if env['PLATFORM'] == 'darwin':
        env.Append(SHLINKFLAGS=['-headerpad_max_install_names'])
    _add_rpath(env)
    _fix_aix_cpp_link(e, cplusplus, 'SHLINKFLAGS')
    return e


def get_bin_environment(envi):
    env= envi.Clone()
    if env['static']:
        if env['CC'] == 'gcc':
            env.Append(LINKFLAGS=['-static'])
            return env
        else:
            print "Static builds only supported with GCC, ignored."
    _add_rpath(env)
    return env


def get_pyext_environment(env, mod_prefix, cplusplus=False):
    """Get a modified environment for building a Python extension.
       `mod_prefix` should be a unique prefix for this module.
       If `cplusplus` is True, additional configuration suitable for a C++
       extension is done."""
    from platform import system
    e = env.Clone()

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
        # The compile and link programs used by python must both produce outputs
        # that are compatible with the compiler we are already using as well
        # as much take all command line options we are already using. As a
        # result, we might as well used the same compiler as before. It would
        # be great to check if they match, but that is kind of hard.
        (opt, basecflags, so)\
            = get_config_vars('OPT', 'BASECFLAGS', 'SO')
        # distutils on AIX can get confused if AIX C but GNU C++ is installed:
        #if platform == 'aix' and cxx == '':
        #    cxx = 'g++'
        #    ldshared = ldshared.replace(' cc_r', ' g++')
        e.Replace(LDMODULESUFFIX=so,CPPFLAGS=basecflags.split() + opt.split())
        #e.Replace(CXX=cxx, LDMODULE=ldshared, SHLINK=ldshared)
        if e['PLATFORM'] is 'posix' and e['rpath']:
            for p in e['LIBPATH']:
                if p[0] is not '#':
                    # append/prepend must match other uses
                    e.Prepend(LINKFLAGS=['-Wl,-rpath,'+p])
        for f in ['NDEBUG']:
            try:
                e['CPPDEFINES'].remove(f)
            except ValueError:
                pass
            except KeyError:
                pass
        # Don't require stack protector stuff on Linux, as this adds a
        # requirement for glibc-2.4:
        # remove the warnings flags because swig produces code which triggers
        # tons of them.
        for v in ['CCFLAGS', 'CPPFLAGS', 'CXXFLAGS']:
            ov= [x for x in e[v]]
            for f in ['-Werror', '-Wall','-O2', '-O3',
                      '-fstack-protector', '-Wstrict-prototypes',
                      '-DNDEBUG']:
                try:
                    ov.remove(f)
                except ValueError:
                    pass
            e[v]=ov
        if platform == 'darwin':
            e.Replace(LDMODULEFLAGS= \
                      '$LINKFLAGS -bundle -flat_namespace -undefined suppress')
        # Don't set link flags on Linux, as that would negate our GNU_HASH check
        elif system() != "Linux":
            e['LDMODULEFLAGS'] = []
            #e['SHLINK'] = e['LDMODULE'] = ldshared
    e.Append(CPPDEFINES=['IMP_SWIG_WRAPPER'])
    e.Append(CPPPATH=[_get_python_include(e)])
    _fix_aix_cpp_link(e, cplusplus, 'SHLINKFLAGS')
    return e

def add_common_variables(vars, package):
    """Add common variables to an SCons Variables object."""
    libdir = '${prefix}/lib'
    if hasattr(os, 'uname') and sys.platform == 'linux2' \
       and os.uname()[-1] == 'x86_64':
        # Install in /usr/lib64 rather than /usr/lib on x86_64 Linux boxes
        libdir += '64'
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
                          libdir + '/python%d.%d/site-packages' \
                          % sys.version_info[0:2], PathVariable.PathAccept))
    vars.Add(PathVariable('pyextdir',
                          'Python extension module installation directory',
                          '${pythondir}', PathVariable.PathAccept))
    vars.Add(PathVariable('docdir', 'Documentation installation directory',
                          '${prefix}/share/doc/%s' % package,
                          PathVariable.PathAccept))
    # Note that destdir should not affect any compiled-in paths; see
    # http://www.gnu.org/prep/standards/html_node/DESTDIR.html
    vars.Add(PathVariable('destdir',
                          'String to prepend to every installed filename',
                          '', PathVariable.PathAccept))
    vars.Add(PackageVariable('python_include',
                             'Directory holding Python include files ' + \
                             '(if unspecified, distutils location is used)',
                             'no'))
    vars.Add(PackageVariable('modeller', 'Location of the MODELLER package',
                             'no'))
    vars.Add(BoolVariable('wine',
                          'Build using MS Windows tools via Wine emulation',
                          False))
    vars.Add(EnumVariable('build',
                          "Set to 'release' for a normal build," \
                          +" 'debug' to disable optimization," \
                          +" or 'fast' to disable most runtime checks," \
                          +" but keep debugging information",
                          "release", ['release', 'debug', 'fast']))
    vars.Add(BoolVariable('linksysv',
                          'Link with old-style SysV, not GNU hash, for ' + \
                          'binary compatibility', False))
    vars.Add('includepath', 'Include search path ' + \
             '(e.g. "/usr/local/include:/opt/local/include")', None)
    vars.Add('libpath', 'Library search path ' + \
             '(e.g. "/usr/local/lib:/opt/local/lib")', None)
    vars.Add('libs', 'Extra libs to add to link commands ' + \
             '(e.g. "efence:pthread")', None)
    vars.Add(BoolVariable('cgal', 'Whether to use the CGAL package', True))
    vars.Add(BoolVariable('rpath',
                          'Add any entries from libpath to library search ' + \
                          'path (rpath) on Linux systems', True))
    vars.Add('ldlibpath', 'Add to the runtime library search path ' +\
             '(LD_LIBRARY_PATH on linux-like systems) for various ' + \
             'build tools and the test cases', None)
    vars.Add('cxxflags', 'Extra cxx flags (e.g. "-fno-rounding -DFOOBAR")',
             None)
    vars.Add('linkflags', 'Extra link flags (e.g. "-lefence")', None)
    vars.Add('path', 'Extra executable path ' + \
             '(e.g. "/opt/local/bin/") to search for build tools', None)
    vars.Add('precommand',
             'A command to be run to wrap program invocations.' + \
             'For example, "valgrind --db-attach=yes --suppressions=valgrind-python.supp"', "")
    vars.Add('pythonpath', 'Extra python path ' + \
             '(e.g. "/opt/local/lib/python-2.5/") to use for tests', None)
    vars.Add(BoolVariable('deprecated',
                          'Build deprecated classes and functions', True))
    vars.Add(BoolVariable('dot',
                          'Use dot from graphviz to lay out graphs in the documentation if available. This produces prettier graphs, but is slow.',
                          True))
    vars.Add(BoolVariable('svn',
                          'True if this build is from an svn version of IMP. If so, SVN version info is added to the provided version number.',
                          True))
    vars.Add(BoolVariable('python', 'Whether to build the python libraries ', True))
    vars.Add(BoolVariable('localmodules', 'Whether to build local modules that are not part of the IMP distribution', False))
    vars.Add(BoolVariable('linktest', 'Test for header defined functions which are not inline', True))
    vars.Add(PathVariable('repository', 'Where to find the source code to build. This is only needed if building in a different directory than the source.', None, PathVariable.PathIsDir))
    vars.Add(BoolVariable('static', 'Whether to only build static libraries. This implies python=False ', False))
    vars.Add(BoolVariable('precompiledheader', 'Whether to use a precompiled header for swig libraries ', False))
    #vars.Add(BoolVariable('noexternaldependencies', 'Do not check files in the provided includepath and libpath for changes.', False))
