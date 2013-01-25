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
import dependency.clang
import platform
import biology
import application
import test
import doc
import run
import data
import utility
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
        posix_env = Environment(platform='posix', **kw)
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
        self.Append(CXXFLAGS=["/MD", "/GR", "/EHsc", "/DNOMINMAX"])

    def _fix_scons_msvc_detect(self):
        """Ensure that MSVC auto-detection finds tools on Wine builds"""
        def _wine_read_reg(value):
            return '/usr/lib/w32comp/Program Files/' + \
                   'Microsoft Visual Studio 10.0'
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
        pythoninclude=utility.get_python_result(env, "import distutils.sysconfig",
                                      "distutils.sysconfig.get_python_inc()")
        #print "include", pythoninclude
        return pythoninclude


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

#If the output is not a terminal, remove the colors
    if env['wine']:
        env = _WineEnvironment(variables=variables,
                              ENV = {'PATH':newpath},
                              *args, **kw)
    else:
        env = Environment(variables=variables,
                          ENV = {'PATH':newpath},
                          *args, **kw)
        #env['PYTHON'] = 'python'
    data.add(env)
    impvariables.update(env, variables)
    colors = {}
    colors['cyan']   = '\033[96m'
    colors['purple'] = '\033[95m'
    colors['blue']   = '\033[94m'
    colors['green']  = '\033[92m'
    colors['yellow'] = '\033[93m'
    colors['red']    = '\033[91m'
    colors['end']    = '\033[0m'
    env['IMP_COLORS']=colors
    if not env['color'] or "TERM" not in os.environ.keys()\
            or os.environ["TERM"] != "xterm":
        for key, value in colors.iteritems():
            colors[key] = ''
    if env['pretty']:
        compile_source_message = '%sCompiling %s$TARGET%s' % \
            (colors['blue'], colors['end'], colors['end'])

        compile_shared_source_message = '%sCompiling shared %s$TARGET%s' % \
            (colors['blue'], colors['end'], colors['end'])

        link_program_message = '%sLinking Program %s$TARGET%s' % \
            (colors['purple'], colors['end'], colors['end'])

        link_library_message = '%sLinking Static Library %s$TARGET%s' % \
            (colors['purple'], colors['end'], colors['end'])

        ranlib_library_message = '%sRanlib Library %s$TARGET%s' % \
            (colors['purple'], colors['end'], colors['end'])

        link_shared_library_message = '%sLinking Shared Library %s$TARGET%s' % \
            (colors['purple'], colors['end'], colors['end'])

        link_module_message = '%sLinking Shared Module %s$TARGET%s' % \
            (colors['purple'], colors['end'], colors['end'])

        java_library_message = '%sCreating Java Archive %s$TARGET%s' % \
            (colors['purple'], colors['end'], colors['end'])

        swig_message = '%sRunning swig on %s$TARGET%s' % \
            (colors['purple'], colors['end'], colors['end'])

        pretty={'CXXCOMSTR': compile_source_message,
                'CCCOMSTR': compile_source_message,
                'SHCCCOMSTR': compile_shared_source_message,
                'SHCXXCOMSTR': compile_shared_source_message,
                'ARCOMSTR': link_library_message,
                'RANLIBCOMSTR': ranlib_library_message,
                'SHLINKCOMSTR': link_shared_library_message,
                'LDMODCOMSTR': link_module_message,
                'LINKCOMSTR': link_program_message,
                'JARCOMSTR': java_library_message,
                'JAVACCOMSTR': compile_source_message,
                'SWIGCOMSTR': swig_message,}
        for p in pretty.keys():
            env[p]=pretty[p]
    #col = colorizer.colorizer()
    #col.colorize(env)
    env['PYTHONPATH'] = '#/build/lib'
    if env.get('pythonpath', None):
        env['PYTHONPATH'] = os.path.pathsep.join(['#/build/lib']+[env['PYTHONPATH']])
    env['all_modules']=[]
    #env.Decider('MD5-timestamp')

    dependency.boost.configure_env_for_auto_link(env)

    # Make Modeller exetype variable available:
    if os.environ.has_key('EXECUTABLE_TYPESVN'):
        env['ENV']['EXECUTABLE_TYPESVN'] = os.environ['EXECUTABLE_TYPESVN']
    # Set empty variables in case the Modeller check fails:
    for mod in ('MODPY', 'EXETYPE'):
        env['MODELLER_' + mod] = ''
    for mod in ('CPPPATH', 'LIBPATH', 'LIBS'):
        env['MODELLER_' + mod] = []
    dependency.compilation.configure_check(env)
    if platform == 'aix':
        # Make sure compilers are in the PATH, so that Python's script for
        # building AIX extension modules can find them:
        e['ENV']['PATH'] += ':/usr/vac/bin'
    #print "cxx", env['CXXFLAGS']
    builddir= Dir("#/build/").abspath
    env.Prepend(CPPPATH=[builddir+"/include"])
    env.Prepend(LIBPATH=[builddir+"/lib"])
    env.Append(BUILDERS={'IMPRun': run.Run})
    # these should be in application, but...
    env.AddMethod(application.IMPApplication)
    env.AddMethod(application.IMPCPPExecutable)
    env.AddMethod(application.IMPCPPExecutables)
    env.AddMethod(application.IMPApplicationTest)
    env.AddMethod(application.IMPPythonExecutable)

    # these should be in systems
    env.AddMethod(biology.IMPSystem)

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
    env.AddMethod(module.IMPModuleGetExpensivePythonTests)
    env.AddMethod(module.IMPModuleGetCPPTests)
    env.AddMethod(module.IMPModuleGetData)
    env.AddMethod(module.IMPModuleGetSources)
    env.AddMethod(module.IMPModuleGetPython)
    env.AddMethod(module.IMPModuleGetSwigFiles)
    env.AddMethod(module.IMPModuleGetBins)
    env.AddMethod(module.IMPModuleGetBenchmarks)
    env.AddMethod(module.IMPModuleBin)
    env.AddMethod(module.IMPModuleBenchmark)
    env.AddMethod(module.IMPModuleDoc)
    env.AddMethod(module.IMPModuleExamples)
    env.AddMethod(module.IMPModuleGetDocs)
    env.AddMethod(doc.IMPPublication)
    env.AddMethod(doc.IMPWebsite)
    env.AddMethod(doc.IMPStandardPublications)
    env.AddMethod(doc.IMPStandardLicense)
    env.Append(BUILDERS={'IMPModuleCPPTest': test.CPPTestHarness})
    env.Append(BUILDERS={'IMPModuleSWIG': module._swig.get_swig_action(env)})
    env.Append(BUILDERS={'IMPModulePatchSWIG': module._swig.PatchSwig})
    env.Append(BUILDERS = {'IMPModuleVersionH': module._version_h.VersionH,
                           'IMPModuleLinkTest': module._link_test.LinkTest})
    if env.get('linkflags', None):
        env.Append(LINKFLAGS=env['linkflags'])
    return env

def _fix_aix_cpp_link(env, cplusplus, linkflags):
    """On AIX things get confused if AIX C but not AIX C++ is installed - AIX C
       options get passed to g++ - so hard code GNU link flags"""
    if cplusplus and 'aixcc' in env['TOOLS'] and 'aixc++' not in env['TOOLS'] \
       and 'g++' in env['TOOLS']:
        slflags = str(env[linkflags])
        env[linkflags] = slflags.replace('-qmkshrobj -qsuppress=1501-218',
                                         '-shared')


def _add_dependency_flags(env, dependencies):
    for d in dependencies:
        dta= data.get_dependency(d)
        if dta.has_key("includepath"):
            for p in dta["includepath"]:
                utility.add_to_include_path(env, p)
        if dta.has_key("libpath"):
            for p in  dta["libpath"]:
                utility.add_to_lib_path(env,p)


def _add_flags(env, extra_modules=[], extra_dependencies=[]):
    if env['IMP_USE_RPATH']:
        dylinkflags=[]
        for p in env['LIBPATH']:
            if p[0] != '#':
                env.Prepend(RPATH=[p])
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
        module_libs.append(d.modules[m].libname)
    # Hack: MSVC will not link against libraries that export no symbols,
    # so don't pass these to the linker
    if env.get('wine', False) and 'imp_compatibility' in module_libs:
        module_libs.remove('imp_compatibility')
    _add_dependency_flags(env, final_dependencies)
    dependency_libs=[]
    for dc in final_dependencies:
        dp=data.get_dependency(dc)
        if dp.has_key("libs"):
            dependency_libs+= dp["libs"]

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
    env = bug_fixes.clone_env(env)
    env.Replace(CXXFLAGS=env['IMP_SHLIB_CXXFLAGS'])
    env.Replace(SHLINKFLAGS=env['IMP_SHLIB_LINKFLAGS'])
    env.Append(CPPDEFINES=[cppdefine, '${VIS_CPPDEFINES}'],
             CXXFLAGS='${VIS_CXXFLAGS}')
    _fix_aix_cpp_link(env, cplusplus, 'SHLINKFLAGS')
    _add_flags(env, extra_modules=extra_modules)
    return env



def get_staticlib_environment(env):
    """Get a modified environment suitable for building shared libraries
       (i.e. using gcc ELF visibility macros or MSVC dllexport/dllimport macros
       to mark dynamic symbols as exported or private). `cppdefine` should be
       the name of a cpp symbol to define to tell MSVC that we are building the
       library (by convention something of the form FOO_EXPORTS).
       If `cplusplus` is True, additional configuration suitable for a C++
       shared library is done."""
    e = bug_fixes.clone_env(env)
    env.Replace(CXXFLAGS=env['IMP_ARLIB_CXXFLAGS'])
    env.Replace(LIBLINKFLAGS=env['IMP_ARLIB_LINKFLAGS'])
    _add_flags(e)
    _fix_aix_cpp_link(e, True, 'LINKFLAGS')
    return e


def get_bin_environment(envi, extra_modules=[], extra_dependencies=[]):
    env= bug_fixes.clone_env(envi)
    env.Replace(CXXFLAGS=env['IMP_BIN_CXXFLAGS'])
    env.Replace(LINKFLAGS=env['IMP_BIN_LINKFLAGS'])
    if env.get('IMP_BIN_CXXFLAGS', None):
        env.Replace(CXXFLAGS=env['IMP_BIN_CXXFLAGS'])
    _add_flags(env, extra_modules=extra_modules,
               extra_dependencies=extra_dependencies)
    if data.get_has_configured_dependency("tcmalloc_heapchecker")\
        and (data.get_dependency("tcmalloc_heapchecker")["ok"]\
        or data.get_dependency("tcmalloc_heapprofiler")["ok"]):
        env.Append(LIBS=["tcmalloc"])
    return env

def get_benchmark_environment(envi, extra_modules=[]):
    extra=[]
    return get_bin_environment(envi, extra_modules+['benchmark'])

def get_test_environment(envi):
    """environment for running config tests"""
    env= bug_fixes.clone_env(envi)
    env.Replace(CXXFLAGS=env['IMP_BIN_CXXFLAGS'])
    env.Replace(LINKFLAGS=env['IMP_BIN_LINKFLAGS'])
    env.Replace(LIBS=utility.get_env_paths(envi, 'libs'))
    if env['IMP_USE_RPATH']:
        dylinkflags=[]
        for p in env['LIBPATH']:
            if p[0] != '#':
                env.Prepend(RPATH=[p])
    env['IMP_OUTER_ENVIRONMENT']= envi
    return env

def get_pyext_environment(env, mod_prefix, cplusplus=True,
                          extra_modules=[]):
    """Get a modified environment for building a Python extension.
       `mod_prefix` should be a unique prefix for this module.
       If `cplusplus` is True, additional configuration suitable for a C++
       extension is done."""
    #print env['CXXFLAGS'], env['CPPPATH']
    e = bug_fixes.clone_env(env)
    e.Replace(CXXFLAGS=env['IMP_PYTHON_CXXFLAGS'])
    e.Replace(LINKFLAGS=[])
    e.Replace(LDMODULEFLAGS=env['IMP_PYTHON_LINKFLAGS'])
    # Set SHLINKFLAGS as well, since some versions of scons use that
    # even when we build a LoadableModule
    e.Replace(SHLINKFLAGS=env['IMP_PYTHON_LINKFLAGS'])

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
    e['LDMODULESUFFIX'] =e['IMP_PYTHON_SO']
    e.Append(CPPDEFINES=['IMP_SWIG_WRAPPER'])
    utility.add_to_include_path(e, _get_python_include(e))
    _fix_aix_cpp_link(e, cplusplus, 'LDMODULEFLAGS')
    #print env['LDMODULEFLAGS']
    _add_flags(e, extra_modules=extra_modules)
    return e

def get_named_environment(env, name, modules, dependencies):
    e = bug_fixes.clone_env(env)
    e['IMP_CURRENT_NAME']=name
    e['IMP_CURRENT_DEPENDENCIES']=dependencies
    e['IMP_CURRENT_MODULES']=modules
    return e

def get_current_dependencies(env):
    return env['IMP_CURRENT_DEPENDENCIES']
def get_current_modules(env):
    return env['IMP_CURRENT_MODULES']

def get_current_name(env):
    return env['IMP_CURRENT_NAME']
