"""Utility functions used by all IMP modules"""

import os.path
import re
import sys
from SCons.Script import *
import bug_fixes
import standards
import dependency.compilation
import module
import dependency
import platform
import biology
import application
import test
import doc
import run
import data
import variables as impvariables
from mypopen import MyPopen
import SCons




class _WineEnvironment(Environment):
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
        self.Append(CXXFLAGS=["/MD", "/GR", "/EHsc"])
        # Disable boost auto-linking, since it sometimes gets the names
        # incorrect or links libraries we didn't use; we manually link
        # libraries we use anyway.
        self.Append(CPPDEFINES=["BOOST_ALL_NO_LIB"])
        self.Append(IMP_PYTHON_CXXFLAGS=["/MD", "/GR", "/EHsc"])

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
        if dependency.gcc.get_is_gcc(env):
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


    if dependency.gcc.get_is_gcc(env):
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
            if dependency.gcc.get_version(env)>= 4.3:
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
            if p[0] != '#':
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


def get_base_environment(variables=None, *args, **kw):
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
        env = _WineEnvironment(variables=variables,
                              ENV = {'PATH':newpath},
                              *args, **kw)
    else:
        env = Environment(variables=variables,
                          ENV = {'PATH':newpath},
                          *args, **kw)
        env['PYTHON'] = 'python'
    data.add(env)
    impvariables.update(env, variables)
    if env['IMP_USE_PLATFORM_FLAGS']:
        _add_platform_flags(env)
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
    dependency.compilation.configure_check(env)
    if platform == 'aix':
        # Make sure compilers are in the PATH, so that Python's script for
        # building AIX extension modules can find them:
        e['ENV']['PATH'] += ':/usr/vac/bin'
    #print "cxx", env['CXXFLAGS']
    env.Prepend(CPPPATH=['#/build/include'])
    env.Prepend(LIBPATH=['#/build/lib'])
    env.Append(BUILDERS={'IMPRun': run.Run})
    # these should be in application, but...
    env.AddMethod(application.IMPApplication)
    env.AddMethod(application.IMPCPPExecutable)
    env.AddMethod(application.IMPApplicationTest)
    env.AddMethod(application.IMPPythonExecutable)

    # these should be in systems
    env.AddMethod(biology.IMPSystem)
    env.AddMethod(biology.IMPSystemHelperModuleBuild)

    # these should be in the module, but this seems to speed things up
    env.AddMethod(module.IMPModuleBuild)
    env.AddMethod(module.IMPModuleLib)
    env.AddMethod(module.IMPModuleInclude)
    env.AddMethod(module.IMPModuleData)
    env.AddMethod(module.IMPModulePython)
    env.AddMethod(module.IMPModuleTest)
    env.AddMethod(module.IMPModuleBuild)
    env.AddMethod(module.IMPModuleGetHeaders)
    env.AddMethod(module.IMPModuleGetExamples)
    env.AddMethod(module.IMPModuleGetExampleData)
    env.AddMethod(module.IMPModuleGetPythonTests)
    env.AddMethod(module.IMPModuleGetCPPTests)
    env.AddMethod(module.IMPModuleGetData)
    env.AddMethod(module.IMPModuleGetSources)
    env.AddMethod(module.IMPModuleGetPython)
    env.AddMethod(module.IMPModuleGetSwigFiles)
    env.AddMethod(module.IMPModuleGetBins)
    env.AddMethod(module.IMPModuleBin)
    env.AddMethod(module.IMPModuleDoc)
    env.AddMethod(module.IMPModuleExamples)
    env.AddMethod(module.IMPModuleGetDocs)
    env.AddMethod(doc.IMPPublication)
    env.AddMethod(doc.IMPWebsite)
    env.AddMethod(doc.IMPStandardPublications)
    env.AddMethod(doc.IMPStandardLicense)
    env.Append(BUILDERS={'IMPModuleCPPTest': test.CPPTestHarness})
    env.Append(BUILDERS={'IMPModuleSWIG': module._swig.SwigIt})
    env.Append(BUILDERS={'IMPModulePatchSWIG': module._swig.PatchSwig})
    env.Append(BUILDERS={'IMPModuleSWIGPreface': module._swig.SwigPreface})
    env.Append(BUILDERS = {'IMPModuleConfigH': module._config_h.ConfigH,
                           'IMPModuleConfigCPP': module._config_h.ConfigCPP,
                           'IMPModuleLinkTest': module._link_test.LinkTest})
    return env

def _fix_aix_cpp_link(env, cplusplus, linkflags):
    """On AIX things get confused if AIX C but not AIX C++ is installed - AIX C
       options get passed to g++ - so hard code GNU link flags"""
    if cplusplus and 'aixcc' in env['TOOLS'] and 'aixc++' not in env['TOOLS'] \
       and 'g++' in env['TOOLS']:
        slflags = str(env[linkflags])
        env[linkflags] = slflags.replace('-qmkshrobj -qsuppress=1501-218',
                                         '-shared')



def _add_link_flags(env, extra_modules=[], extra_dependencies=[]):
    modules=extra_modules+env['IMP_CURRENT_MODULES']
    dependencies=env['IMP_CURRENT_DEPENDENCIES']+extra_dependencies
    all_dependencies=dependencies
    d= data.get(env)
    all_modules=[]
    for m in modules:
        if m not in all_modules:
            all_modules= all_modules+[m]+d.modules[m].modules
    final_modules=[]
    for i,m in enumerate(all_modules):
        if not m in all_modules[i+1:]:
            final_modules.append(m)
    for m in final_modules:
        all_dependencies= all_dependencies+d.modules[m].dependencies
    final_dependencies=[]
    for i,dc in enumerate(all_dependencies):
        if not dc in all_dependencies[i+1:]:
            final_dependencies.append(dc)
    module_libs=[]
    for m in final_modules:
        if m=='kernel':
            module_libs.append('imp')
        else:
            module_libs.append('imp_'+m)
    dependency_libs=[]
    for dc in final_dependencies:
        dependency_libs+= d.dependencies[dc].libs
    env.Append(LIBS=module_libs)
    env.Append(LIBS=dependency_libs)

def get_sharedlib_environment(env, cppdefine, cplusplus=False,
                              extra_modules=[]):
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
    _add_link_flags(e, extra_modules=extra_modules)
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


def get_bin_environment(envi, extra_modules=[]):
    env= bug_fixes.clone_env(envi)
    env.Replace(LINKFLAGS=env['IMP_BIN_LINKFLAGS'])
    _add_link_flags(env, extra_modules=extra_modules)
    return env


def get_pyext_environment(env, mod_prefix, cplusplus=True,
                          extra_modules=[]):
    """Get a modified environment for building a Python extension.
       `mod_prefix` should be a unique prefix for this module.
       If `cplusplus` is True, additional configuration suitable for a C++
       extension is done."""
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
    _add_link_flags(e, extra_modules=extra_modules)
    return e

def get_named_environment(env, name, modules, dependencies):
    e = bug_fixes.clone_env(env)
    e['IMP_CURRENT_NAME']=name
    e['IMP_CURRENT_DEPENDENCIES']=dependencies
    e['IMP_CURRENT_MODULES']=modules
    return e

def get_current_name(env):
    return env['IMP_CURRENT_NAME']
