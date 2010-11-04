"""Utility functions used by all IMP modules"""

import os.path
import re
import sys
from SCons.Script import *
import hierarchy
import symlinks
import bug_fixes
import standards
import compilation
import imp_module
import dependency
import gcc
import swig
import platform
from mypopen import MyPopen

__all__ = ["add_common_variables", "MyEnvironment", "get_pyext_environment",
           "get_sharedlib_environment", "get_staticlib_environment"]

import SCons

def _propagate_variables(env):
    """enforce dependencies between variables"""
    env['IMP_BUILD_STATIC']= env['static']
    env['IMP_BUILD_DYNAMIC']= env['dynamic']
    env['IMP_PROVIDE_PYTHON']= env['python']
    env['IMP_USE_PLATFORM_FLAGS']= env['platformflags']
    env['IMP_USE_RPATH']= env['rpath']
    if env['pythonsosuffix'] != 'default':
        env['IMP_PYTHON_SO']=env['pythonsosuffix']
    elif env['IMP_PROVIDE_PYTHON'] and not env['IMP_USE_PLATFORM_FLAGS']:
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
    if gcc.get_is_gcc(env):
        env['IMP_USE_PCH']=env['precompiledheader']
    else:
        env['IMP_USE_PCH']=False
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
    if env.get('cxxflags', None):
        env.Append(CXXFLAGS = env['cxxflags'].split())
    else:
        env.Append(CXXFLAGS=[])

    if env.get('pythoncxxflags', None):
        env.Append(IMP_PYTHON_CXXFLAGS = env['pythoncxxflags'].split())
    elif env.get('cxxflags', None):
        env.Append(IMP_PYTHON_CXXFLAGS = env['cxxflags'].split())
    else:
        env.Append(IMP_PYTHON_CXXFLAGS=[])

    if env.get('linkflags', None):
        env.Append(IMP_LINKFLAGS=env['linkflags'].split())
    else:
        env.Append(IMP_LINKFLAGS=[])
    if env.get('pythonlinkflags', None):
        env.Append(IMP_PYTHON_LINKFLAGS=env['pythonlinkflags'].split())
    else:
        env.Append(IMP_PYTHON_LINKFLAGS=[])

    if env.get('shliblinkflags', None):
        env.Append(IMP_SHLIB_LINKFLAGS=env['shliblinkflags'].split())
    else:
        env.Append(IMP_SHLIB_LINKFLAGS=[])

    if env.get('arliblinkflags', None):
        env.Append(IMP_ARLIB_LINKFLAGS=env['arliblinkflags'].split())
    else:
        env.Append(IMP_ARLIB_LINKFLAGS=[])


    if env.get('binlinkflags', None):
        env.Append(IMP_BIN_LINKFLAGS=env['binlinkflags'].split())
    else:
        env.Append(IMP_BIN_LINKFLAGS=[])

    if env.get('includepath') is not None:
        env['includepath'] = [os.path.abspath(x) for x in \
                          env['includepath'].split(os.path.pathsep)]
        env.Prepend(CPPPATH=env['includepath'])
    else:
        env.Append(CPPPATH=[])

    if env.get('libpath') is not None:
        env.Prepend(LIBPATH=[os.path.abspath(x) for x in \
                             env['libpath'].split(os.path.pathsep)])
    else:
        env.Append(LIBPATH=[])
    if env.get('libs') is not None:
        libs= env['libs'].split(":")
        env.Append(LIBS=libs)
    else:
        env.Append(LIBS=[])

    if env.get('ldlibpath') is not None:
        env['ENV']['LD_LIBRARY_PATH'] = env['ldlibpath']


def GetInstallDirectory(env, varname, *subdirs):
    """Get a directory to install files in. The top directory is env[varname],
       prefixed with env['destdir']. The full directory is constructed by
       adding any other function arguments as subdirectories."""
    destdir = env.subst(env['destdir'])
    installdir = env.subst(env[varname])
    if destdir != '' and not os.path.isabs(installdir):
        print >> sys.stderr, "Install directory %s (%s) must be an absolute path,\n" \
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
            print >> sys.stderr, "ERROR: Wine is supported only on Linux systems"
            Exit(1)
        self._fix_scons_msvc_detect()

        Environment.__init__(self, platform=platform, CC=CC, LINK=LINK, **kw)
        posix_env = Environment(platform='posix')
        self['SHLIBPREFIX'] = self['LIBLINKPREFIX'] = self['LIBPREFIX'] = 'lib'
        self['WINDOWSEXPPREFIX'] = 'lib'
        self['LIBSUFFIX'] = '.lib'
        self['PSPAWN'] = posix_env['PSPAWN']
        self['SPAWN'] = posix_env['SPAWN']
        self['SHELL'] = posix_env['SHELL']
        self['ENV'] = posix_env['ENV']
        self['PYTHON'] = 'w32python'
        # Use / rather than \ path separator:
        self['LINKCOM'] = self['LINKCOM'].replace('.windows', '')
        # Make sure we get the same Windows C/C++ library as Modeller, and
        # enable C++ exception handling
        self.Append(CFLAGS="/MD")
        self.Append(CXXFLAGS=["/MD", "/GR", "/GX"])
        self.Append(IMP_PYTHON_CXXFLAGS=["/MD", "/GR", "/GX"])

    def _fix_scons_msvc_detect(self):
        """Ensure that MSVC auto-detection finds tools on Wine builds"""
        def _wine_read_reg(value):
            return '/usr/lib/w32comp/Program Files/' + \
                   'Microsoft Visual Studio .NET 2003'
        try:
            import SCons.Tool.MSCommon.common
        except ImportError:
            return # Older versions of scons don't have this module
        SCons.Tool.MSCommon.common.read_reg = _wine_read_reg


pythoninclude=None
def _get_python_include(env):
    global pythoninclude
    """Get the directory containing Python.h"""
    if env['pythoninclude']:
        return env['pythoninclude']
    elif env['wine']:
        return '/usr/lib/w32comp/w32python/2.6/include/'
    elif pythoninclude:
        return pythoninclude
    else:
        oldpath = os.environ['PATH']
        os.environ['PATH']=env['ENV']['PATH']
        p = MyPopen('python')
        os.environ['PATH'] = oldpath
        print >> p.stdin, """
import distutils.sysconfig
print distutils.sysconfig.get_python_inc()
"""
        p.stdin.close()
        p.wait()

        pythoninclude = p.stdout.read().split('\n')[0]
        return pythoninclude


def _add_platform_flags(env):
    """Add compiler flags for release builds, if requested"""
    if not env['IMP_USE_PLATFORM_FLAGS']:
        raise ValueError("platform flags is false")

    #make sure they are all there
    env.Append(CPPPATH=[])
    env.Append(CXXFLAGS=[])
    env.Append(LINKFLAGS=[])
    env.Append(LIBPATH=[])

    if not env.get('wine', None):
        from distutils.sysconfig import get_config_vars
        # The compile and link programs used by python must both produce outputs
        # that are compatible with the compiler we are already using as well
        # as much take all command line options we are already using. As a
        # result, we might as well used the same compiler as before. It would
        # be great to check if they match, but that is kind of hard.
        (opt, cflags, so) = get_config_vars('OPT', 'BASECFLAGS', 'SO')
        env['IMP_PYTHON_SO']=so
        if gcc.get_is_gcc(env):
            basecflags=[x for x in opt.split()+cflags.split() \
                        if x not in ['-Werror', '-Wall','-O2', '-O3',
                                     '-fstack-protector', '-Wstrict-prototypes',
                                     '-g', '-dynamic', '-DNDEBUG',
                                     "-fwrapv", "-fno-strict-aliasing"]]
                    #total.append(v)
        else:
            basecflags= opt.split()+cflags.split()
        env.Append(CXXFLAGS=basecflags)

    if env['PLATFORM'] == 'darwin':
        env.Append(IMP_PYTHON_LINKFLAGS=
                ['-bundle', '-flat_namespace', '-undefined', 'suppress'])


    if gcc.get_is_gcc(env):
        env.Append(CXXFLAGS=["-Wall", "-Wno-deprecated"])
        env.Append(CXXFLAGS=["-Woverloaded-virtual"])
        if env['build'] == 'fast':
            env.Append(CXXFLAGS=["-O3", "-fexpensive-optimizations",
                                 "-ffast-math", "-ftree-vectorize",
                                 '-ffinite-math-only',
                                 '-fstrict-aliasing',
                                 '-fno-trapping-math',
                                 '-fno-signaling-nans',
                                 '-fno-float-store'])
            if gcc.get_version(env)>= 4.3:
                env.Append(CXXFLAGS=['-fno-signed-zeros',
                                     '-freciprocal-math',
                                     '-fassociative-math'])
        elif env['build'] == 'release':
            env.Append(CXXFLAGS=["-O2"])
        elif env['build'] == 'debug':
            env.Append(CXXFLAGS=["-g"])
            env.Append(LINKFLAGS=["-g"])
        try:
            env['SHLINKFLAGS'] = [ x.replace('-no_archive', '') for x in env['SHLINKFLAGS']]
        except ValueError:
            pass
        env.Replace(IMP_PYTHON_CXXFLAGS=[x for x in env['IMP_PYTHON_CXXFLAGS']+env['CXXFLAGS']
                                     if x not in ['-Wall', '-Wextra', '-Wformat', '-O3', '-O2']])
    if env['IMP_USE_RPATH']:
        dylinkflags=[]
        for p in env['LIBPATH']:
            if p[0] is not '#':
                env.Prepend(IMP_SHLIB_LINKFLAGS=['-Wl,-rpath,'+p])
                env.Prepend(IMP_BIN_LINKFLAGS=['-Wl,-rpath,'+p])
        #env.Prepend(LIBLINKFLAGS=['-Wl,-rpath-link,'+Dir("#/build/lib").abspath])
    env.Prepend(IMP_BIN_LINKFLAGS=env['IMP_LINKFLAGS'])
    env.Prepend(IMP_BIN_LINKFLAGS=env['LINKFLAGS'])
    env.Prepend(IMP_SHLIB_LINKFLAGS=env['IMP_LINKFLAGS'])
    env.Prepend(IMP_SHLIB_LINKFLAGS=env['SHLINKFLAGS'])
    if env['IMP_BUILD_STATIC']:
        env.Prepend(IMP_ARLIB_LINKFLAGS=env['IMP_LINKFLAGS'])
        env.Prepend(IMP_ARLIB_LINKFLAGS=env['LINKFLAGS'])
        env.Append(IMP_BIN_LINKFLAGS=['-static'])
    env.Prepend(IMP_PYTHON_LINKFLAGS=env['IMP_LINKFLAGS'])
    env.Prepend(IMP_PYTHON_LINKFLAGS=env['LDMODULEFLAGS'])
    if env['PLATFORM'] == 'darwin':
        env.Append(IMP_SHLIB_LINKFLAGS=['-headerpad_max_install_names'])
        env.Append(IMP_PYTHON_LINKFLAGS=['-headerpad_max_install_names'])
        env.Append(IMP_BIN_LINKFLAGS=['-headerpad_max_install_names'])


def MyEnvironment(variables=None, *args, **kw):
    """Create an environment suitable for building IMP modules"""
    #import colorizer
    # First make a dummy environment in order to evaluate all variables, since
    # env['wine'] will tell us which 'real' environment to create:
    env = Environment(tools=[], variables=variables)
    #variables.Update(env)
    newpath = env['ENV']['PATH']
    if env.get('path') is not None:
        newpath = env['path'] + os.path.pathsep + newpath
    envargs={'PATH':newpath}
    if env['wine']:
        env = WineEnvironment(variables=variables,
                              ENV = {'PATH':newpath},
                              *args, **kw)
    else:
        env = Environment(variables=variables,
                          ENV = {'PATH':newpath},
                          *args, **kw)
        env['PYTHON'] = 'python'
    variables.Update(env)
    _propagate_variables(env)
    if env['IMP_USE_PLATFORM_FLAGS']:
        _add_platform_flags(env)

    env.AddMethod(symlinks.LinkInstall)
    env.AddMethod(symlinks.LinkInstallAs)
    env.AddMethod(hierarchy.InstallHierarchy)
    env.AddMethod(GetInstallDirectory)
    #col = colorizer.colorizer()
    #col.colorize(env)
    env['PYTHONPATH'] = '#/build/lib'
    if env.get('pythonpath', None):
        env['PYTHONPATH'] = os.path.pathsep.join(['#/build/lib']+[env['PYTHONPATH']])
    env['all_modules']=[]
    env.Decider('MD5-timestamp')

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
        print  >> sys.stderr,"Warning: Could not find 'svnversion' binary in path"
    compilation.configure_check(env)
    if platform == 'aix':
        # Make sure compilers are in the PATH, so that Python's script for
        # building AIX extension modules can find them:
        e['ENV']['PATH'] += ':/usr/vac/bin'
    #print "cxx", env['CXXFLAGS']
    env.Prepend(CPPPATH=['#/build/include'])
    env.Prepend(LIBPATH=['#/build/lib'])
    env.Append(BUILDERS={'IMPRun': run.Run})
    env.Append(BUILDERS={'IMPColorizePython': examples.ColorizePython})
    env.Append(BUILDERS = {'IMPGeneratePCH': pch.GeneratePCH,
                           'IMPBuildPCH': pch.BuildPCH})
    env.AddMethod(imp_module.get_module_ok)
    env.AddMethod(imp_module.get_module_modules)
    env.AddMethod(imp_module.get_module_python_modules)
    env.AddMethod(imp_module.get_module_dependencies)
    env.AddMethod(imp_module.get_module_version)
    env.AddMethod(imp_module.get_found_modules)
    env.AddMethod(dependency.get_found_dependencies)
    env.AddMethod(dependency.add_dependency_link_flags)
    env.AddMethod(dependency.get_dependency_libs)
    env.AddMethod(dependency.get_all_known_dependencies)
    env.AddMethod(dependency.get_dependency_ok)

    # these should be in the module, but this seems to speed things up
    env.AddMethod(imp_module.IMPModuleLib)
    env.AddMethod(imp_module.IMPModuleInclude)
    env.AddMethod(imp_module.IMPModuleData)
    env.AddMethod(imp_module.IMPModulePython)
    env.AddMethod(imp_module.IMPModuleTest)
    env.AddMethod(imp_module.IMPModuleBuild)
    env.AddMethod(imp_module.IMPModuleGetHeaders)
    env.AddMethod(imp_module.IMPModuleGetExamples)
    env.AddMethod(imp_module.IMPModuleGetExampleData)
    env.AddMethod(imp_module.IMPModuleGetPythonTests)
    env.AddMethod(imp_module.IMPModuleGetCPPTests)
    env.AddMethod(imp_module.IMPModuleGetData)
    env.AddMethod(imp_module.IMPModuleGetSources)
    env.AddMethod(imp_module.IMPModuleGetPython)
    env.AddMethod(imp_module.IMPModuleGetSwigFiles)
    env.AddMethod(imp_module.IMPModuleGetBins)
    env.AddMethod(imp_module.IMPModuleBin)
    env.AddMethod(imp_module.IMPModuleDoc)
    env.AddMethod(imp_module.IMPModuleExamples)
    env.AddMethod(imp_module.IMPModuleGetDocs)
    env.AddMethod(modpage.Publication)
    env.AddMethod(modpage.Website)
    env.AddMethod(modpage.StandardPublications)
    env.AddMethod(modpage.StandardLicense)
    env.Append(BUILDERS={'IMPModuleRunTest': test.UnitTest})
    env.Append(BUILDERS={'IMPModuleCPPTest': test.CPPTestHarness})
    env.Append(BUILDERS={'IMPModuleExamplesDox': examples.MakeDox})
    env.Append(BUILDERS={'IMPModuleSWIG': swig.SwigIt})
    env.Append(BUILDERS={'IMPModulePatchSWIG': swig.PatchSwig})
    env.Append(BUILDERS={'IMPModuleSWIGPreface': swig.SwigPreface})
    env.Append(BUILDERS={'IMPModuleMakeModPage': modpage.MakeModPage})
    env.Append(BUILDERS = {'IMPModuleConfigH': config_h.ConfigH,
                           'IMPModuleConfigCPP': config_h.ConfigCPP,
                           'IMPModuleLinkTest': link_test.LinkTest})
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
    e = bug_fixes.clone_env(env)
    e.Append(CPPDEFINES=[cppdefine, '${VIS_CPPDEFINES}'],
             CXXFLAGS='${VIS_CXXFLAGS}')
    e.Replace(SHLINKFLAGS=env['IMP_SHLIB_LINKFLAGS'])
    _fix_aix_cpp_link(e, cplusplus, 'SHLINKFLAGS')
    return e


def get_staticlib_environment(env):
    """Get a modified environment suitable for building shared libraries
       (i.e. using gcc ELF visibility macros or MSVC dllexport/dllimport macros
       to mark dynamic symbols as exported or private). `cppdefine` should be
       the name of a cpp symbol to define to tell MSVC that we are building the
       library (by convention something of the form FOO_EXPORTS).
       If `cplusplus` is True, additional configuration suitable for a C++
       shared library is done."""
    e = bug_fixes.clone_env(env)
    e.Replace(LIBLINKFLAGS=env['IMP_ARLIB_LINKFLAGS'])
    _fix_aix_cpp_link(e, True, 'LINKFLAGS')
    return e


def get_bin_environment(envi):
    env= bug_fixes.clone_env(envi)
    env.Replace(LINKFLAGS=env['IMP_BIN_LINKFLAGS'])
    return env


def get_pyext_environment(env, mod_prefix, cplusplus=False):
    """Get a modified environment for building a Python extension.
       `mod_prefix` should be a unique prefix for this module.
       If `cplusplus` is True, additional configuration suitable for a C++
       extension is done."""
    import copy
    e = bug_fixes.clone_env(env)

    e['LDMODULEPREFIX'] = ''
    # We're not going to link against the extension, so don't need a Windows
    # import library (.lib file):
    e['no_import_lib'] = 1
    platform = e['PLATFORM']
    if e['wine']:
        # Directory containing python26.lib:
        e.Append(LIBPATH=['/usr/lib/w32comp/w32python/2.6/lib/'])
        e['SHLIBSUFFIX']=e['IMP_PYTHON_SO']
        # Have to set SHLIBSUFFIX and PREFIX on Windows otherwise the
        # mslink tool complains
        e['SHLIBPREFIX'] = ''
    e.Replace(LDMODULEFLAGS=env['IMP_PYTHON_LINKFLAGS'])
    e['LDMODULESUFFIX'] =e['IMP_PYTHON_SO']
    #print e['LDMODULEFLAGS']
    e.Replace(CXXFLAGS=e['IMP_PYTHON_CXXFLAGS'])
    #e['CXXFLAGS']=cxxs
    e.Append(CPPDEFINES=['IMP_SWIG_WRAPPER'])
    e.Append(CPPPATH=[_get_python_include(e)])
    _fix_aix_cpp_link(e, cplusplus, 'LDMODULEFLAGS')
    #print env['LDMODULEFLAGS']
    return e



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
    vars.Add(EnumVariable('build',
                          "Set to 'release' for a normal build," \
                          +" 'debug' to disable optimization," \
                          +" or 'fast' to disable most runtime checks," \
                          +" but keep debugging information",
                          "release", ['release', 'debug', 'fast']))
    vars.Add(EnumVariable('endian',
                          "The endianness of the platform. \"auto\" will determine it automatically.",
                          "auto", ['auto', 'big', 'little']))
    vars.Add(BoolVariable('linksysv',
                          'Link with old-style SysV, not GNU hash, for ' + \
                          'binary compatibility', False))
    vars.Add('includepath', 'Include search path ' + \
             '(e.g. "/usr/local/include:/opt/local/include")', None)
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

    vars.Add('linkflags', 'Link flags for all linking (e.g. "-lefence"). See pythonlinkflags, arliblinkflags, shliblinkflags.', None)
    vars.Add('pythonlinkflags', 'Link flags for linking python libraries (e.g. "-lefence")', "")
    vars.Add('arliblinkflags', 'Link flags for linking static libraries (e.g. "-lefence")', "")
    vars.Add('shliblinkflags', 'Link flags for linking shared libraries (e.g. "-lefence")', "")
    vars.Add('binlinkflags', 'Link flags for linking executables (e.g. "-lefence")', "")
    vars.Add('path', 'Extra executable path ' + \
             '(e.g. "/opt/local/bin/") to search for build tools', None)
    vars.Add('precommand',
             'A command to be run to wrap program invocations.' + \
             'For example, "valgrind --db-attach=yes --suppressions=valgrind-python.supp"', "")
    vars.Add('pythonpath', 'Extra python path ' + \
             '(e.g. "/opt/local/lib/python-2.5/") to use for tests', None)
    vars.Add('boostversion', 'The version of boost. If this is not none, the passed version is used and checks are not done. The version should look like "104200" for Boost "1.42".', None)
    vars.Add(BoolVariable('platformflags',
                          'If true, add any compiler and linker arguments that might be needed/desired. If false, only used passed flags (eg only the values in "cxxflags", "linkflags" etc).',
                          True))
    vars.Add(BoolVariable('deprecated',
                          'Build deprecated classes and functions', True))
    vars.Add('pythonsosuffix', 'The suffix for the python libraries.', 'default')
    vars.Add(BoolVariable('dot',
                          'Use dot from graphviz to lay out graphs in the documentation if available. This produces prettier graphs, but is slow.',
                          True))
    vars.Add(BoolVariable('svn',
                          'True if this build is from an svn version of IMP. If so, SVN version info is added to the provided version number.',
                          True))
    vars.Add(BoolVariable('python', 'Whether to build the python libraries ', True))
    vars.Add(BoolVariable('localmodules', 'Whether to build local modules that are not part of the IMP distribution', False))
    vars.Add(BoolVariable('linktest', 'Test for header defined functions which are not inline', True))
    vars.Add(PathVariable('repository', 'Where to find the source code to build. This is only needed if building in a different directory than the source.', None, PathVariable.PathAccept)) #PathIsDir
    vars.Add(BoolVariable('static', 'Whether to build static libraries.', False))
    vars.Add(BoolVariable('dynamic', 'Whether to build dynamic libraries (needed for python support).', True))
    vars.Add(BoolVariable('precompiledheader', 'Whether to use a precompiled header for swig libraries ', False))
    vars.Add('disabledmodules', 'A colon-separated list of modules to disable.', '')
    vars.Add(BoolVariable('fastlink', 'Scons does not handle shared libraries properly by default and relinks everything every time one changes. If fastlink it true, then we work around this. Fastlink can only be set to true when on linux or mac and when there is no static linking. These preconditions are not currently checked. ', False))
    #vars.Add(BoolVariable('noexternaldependencies', 'Do not check files in the provided includepath and libpath for changes.', False))
