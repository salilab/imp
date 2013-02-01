"""Tools and Builders for IMP modules. See `IMPModule` for more information."""

import os.path
import sys
import scons_tools.pyscanner
import _swig
import _header
import _link_test
import _standards
import _config_h
import scons_tools.bug_fixes
import scons_tools.run
import scons_tools.dependency
import scons_tools.doc
import scons_tools.environment
import scons_tools.examples
import scons_tools.install
import scons_tools.utility
import scons_tools.paths as stp
import scons_tools.bins as stb

from SCons.Script import Builder, File, Action, Glob, Return, Dir, Move, Copy, Scanner
from SCons.Scanner import C as CScanner

def _get_module_name(env):
    return scons_tools.environment.get_current_name(env)


def _get_module_full_name(env):
    name= _get_module_name(env)
    if name=="kernel":
        name="IMP"
    else:
        name="IMP."+name
    return name


def _get_module_path(env):
    module= _get_module_name(env)
    name=module
    if name=="kernel":
        name=""
    return name

def _get_module_data(env, module=None):
    if not module:
        module= _get_module_name(env)
    return scons_tools.data.get(env).modules[module]

def _get_module_has_data(env):
    #print _get_module_data(env).data
    return _get_module_data(env).data

def _get_module_alias(env):
    return _get_module_data(env).alias

def _get_found_modules(env, modules):
    ret=[]
    for m in modules:
        if _get_module_data(env, m).ok:
            ret.append(m)
    return ret

def _get_module_modules(env):
    return _get_module_data(env).modules

def _get_module_unfound_modules(env):
    return _get_module_data(env).unfound_modules


def _get_module_version(env):
    return _get_module_data(env).version


def _get_module_unfound_dependencies(env):
    return _get_module_data(env).unfound_dependencies

def _get_module_dependencies(env):
    return _get_module_data(env).dependencies

def _get_module_direct_dependencies(env):
    return _get_module_data(env).direct_dependencies


def _set_module_links(env, links):
    env['IMP_MDLE_LINKS']=links


def _get_module_variables(env):
    """Make a map which can be used for all string substitutions"""
    return env['IMP_MODULE_VARS']


def IMPModuleLib(envi, files=[]):
    """Build, and optionally also install, an IMP module's C++
       shared library. This is only available from within an environment
       created by `IMPSharedLibraryEnvironment`."""
    if envi["IMP_PASS"] != "BUILD":
        return
    if files != []:
        print >> sys.stderr, "WARNING, specifying sources is no longer supported"
    vars= _get_module_variables(envi)
    module = _get_module_name(envi)
    module_libname =_get_module_variables(envi)['module_libname']
    version= _get_module_version(envi)
    data= scons_tools.data.get(envi).modules[_get_module_name(envi)]
    prefix=vars['module_libname']
    if prefix=="imp":
        prefix="imp_kernel"
    #env.AlwaysBuild(version)
    build=[]
    sources=scons_tools.paths.get_matching_source(envi, ["*.cpp", "internal/*.cpp"])
    build_sources=scons_tools.paths.get_matching_build_files(envi,
                                                             ["src/"+module+"/*.cpp"],
        ondisk=True)
    if envi['percppcompilation']=="yes"\
           or module in envi['percppcompilation'].split(":"):
        allf=sources\
            + build_sources
        if envi['build']=="debug" and envi['linktest']:
            link0=envi.IMPModuleLinkTest(target=[stp.get_build_source_file(envi,
                                                                           'link_0.cpp',
                                                                           module)],
                                          source=[])
            link1=envi.IMPModuleLinkTest(target=[stp.get_build_source_file(envi,
                                                                           'link_1.cpp',
                                                                            module)],
                                          source=[])
            allf= allf+link0+link1
    else:
        allcpp=File("#/build/src/%s_all.cpp"%module)
        # scons doesn't get the dependencies right since it can check all.cpp before the
        # file exists
        allf= [allcpp]+build_sources
        if envi['build']=="debug" and envi['linktest']:
            link1=envi.IMPModuleLinkTest(target=[stp.get_build_source_file(envi,
                                                                           'link.cpp',
                                                                           module)],
                                          source=[])
            allf= allf+link1
    if envi['IMP_BUILD_STATIC']:
        env= scons_tools.environment.get_staticlib_environment(envi)
        sl= env.StaticLibrary('#/build/lib/%s' % module_libname,
                              allf)
        scons_tools.data.get(env).add_to_alias(_get_module_alias(env), sl[0])
    if envi['IMP_BUILD_DYNAMIC']:
        env = scons_tools.environment.get_sharedlib_environment(envi, '%(EXPORT)s_EXPORTS' % vars,
                                    cplusplus=True)
        sl=env.SharedLibrary('#/build/lib/%s' % module_libname,
                                       allf )
        scons_tools.data.get(env).add_to_alias(_get_module_alias(env), sl[0])
        scons_tools.utility.postprocess_lib(env, sl)


def IMPModuleInclude(env, files):
    """Install the given header files, plus any auto-generated files for this
       IMP module."""
    if env["IMP_PASS"] != "BUILD":
        return
    vars=_get_module_variables(env)
    module= _get_module_name(env)
    moduleinclude= vars['module_include_path']
    # Generate config header and SWIG equivalent
    data= scons_tools.data.get(env)
    deps= _get_module_dependencies(env)
    signature=_get_module_unfound_dependencies(env)\
        + deps\
        + _get_found_modules(env, _get_module_modules(env))\
        + [_get_module_has_data(env)]

def IMPModuleData(env, files):
    """Install the given data files for this IMP module."""
    if env["IMP_PASS"] != "BUILD":
        return
    dta=scons_tools.data.get(env).modules[_get_module_name(env)]
    dta.data=True


def IMPModuleExamples(env, example_files=None, data_files=None,
                      example_modules=[]):
    if env["IMP_PASS"]=="RUN":
        module= _get_module_name(env)
        example_pyfiles=stp.get_matching_source(env, ["*.py"])+stp.get_matching_build(env, ["test/%s/cpp_examples_test.py"%module])
        example_cppfiles=stp.get_matching_source(env, ["*.cpp"])
        runable=[File(x) for x in example_pyfiles
                 if str(x).find("fragment")==-1]
        deps=[]
        if len(example_cppfiles) > 0:
            prgs= stb.handle_bins(env, example_cppfiles,
                                  stp.get_build_test_dir(env, module),
                                  # link in all modules so as not to have to bother with dependencies
                                        extra_modules=[module]+example_modules)
            deps.extend(prgs)
        if len(runable)>0:
            tests = scons_tools.test.add_tests(env,
                                               source=runable,
                                               dependencies=deps,
                                               type='example')

def IMPModuleBin(env, files):
    if env["IMP_PASS"] != "BUILD":
        return
    prgs=stb.handle_bins(env, files,
                         stp.get_build_bin_dir(env, _get_module_name(env)),
                         extra_modules=[_get_module_name(env)])
    scons_tools.data.get(env).add_to_alias(_get_module_alias(env), prgs)

def IMPModuleBenchmark(env, files):
    # don't disable benchmark by checking for it before it is sourced
    # compatibility and base must not have benchmarks
    if env["IMP_PASS"] != "BUILD" or len(files)==0:
        return
    prgs, bmarks=stb.handle_benchmarks(env, files,
                                       stp.get_build_benchmark_dir(env, _get_module_name(env)),
                                       extra_modules=[_get_module_name(env)])
    scons_tools.data.get(env).add_to_alias(_get_module_alias(env)+"-benchmarks", bmarks)
    scons_tools.data.get(env).add_to_alias(_get_module_alias(env), prgs)

def IMPModulePython(env, swigfiles=[], pythonfiles=[]):
    """Build and install an IMP module's Python extension and the associated
       wrapper file from a SWIG interface file. This is only available from
       within an environment created by `IMPPythonExtensionEnvironment`."""
    if env["IMP_PASS"] != "BUILD" or env["python"]=="no":
        return
    module =_get_module_name(env)
    vars=_get_module_variables(env)
    data=scons_tools.data.get(env)
    moduledata=data.modules[_get_module_name(env)]
    alldata= scons_tools.data.get(env).modules
    penv = scons_tools.environment.get_pyext_environment(env, module.upper(),
                                                         cplusplus=True,
                                                         extra_modules=[module])
    #penv.Decider('timestamp-match')
    versions=[]
    prefix=vars['module_pylibname'][1:]
    if prefix=="IMP":
        prefix="IMP_kernel"
    produced=File("#/build/lib/"+vars['module_include_path']+"/__init__.py")
    version=_get_module_version(penv)
    cppin=stp.get_build_swig_source_file(penv,
                                    "wrap.cpp-in", module)
    hin=stp.get_build_swig_source_file(penv,
                                  "wrap.h-in", module)
    swigr=penv.IMPModuleSWIG(target=[produced,
                                     cppin, hin],
                             source=[File("#/build/swig/"+prefix+".i")])
    #print "Moving", produced.path, "to", dest.path
    cppf=stp.get_build_swig_source_file(penv,
                                   "wrap.cpp", module)
    hf=stp.get_build_swig_source_file(penv,
                                 "wrap.h", module)
    patched=penv.IMPModulePatchSWIG(target=[cppf],
                                    source=[cppin])
    hpatched=penv.IMPModulePatchSWIG(target=[hf],
                                     source=[hin])
    penv.Requires(patched, hpatched)
    lpenv= scons_tools.bug_fixes.clone_env(penv)
    lpenv.Append(CPPDEFINES=["IMP_SWIG"])
    buildlib = lpenv.LoadableModule("#/build/lib/"+vars["module_pylibname"],
                                    patched) #SCANNERS=scanners
    data.add_to_alias(_get_module_alias(env), buildlib[0])
    if "kernel" in _get_module_dependencies(env):
        # all python support needs kernel, silly design to put it in the base
        # namespace/python module
        env.Requires(data.get_alias(module),
                          data.get_alias("kernel"))
    scons_tools.utility.postprocess_lib(penv, buildlib)

def IMPModuleGetExamples(env):
    return []

def IMPModuleGetExampleData(env):
    return []

def IMPModuleGetPythonTests(env):
    return []
def IMPModuleGetCPPTests(env):
    return []
def IMPModuleGetExpensivePythonTests(env):
    return []
def IMPModuleGetExpensiveCPPTests(env):
    return []

def IMPModuleGetHeaders(env):
    return []

def IMPModuleGetSwigFiles(env):
    return []

def IMPModuleGetPython(env):
    return []

def IMPModuleGetSources(env):
    return []

def IMPModuleGetData(env):
    return []

def IMPModuleGetBins(env):
    return stp.get_matching_source(env, ["*.cpp", "*.py"])

def IMPModuleGetBenchmarks(env):
    return stp.get_matching_source(env, ["*.cpp", "*.py"])

def IMPModuleGetDocs(env):
    files=stp.get_matching_source(env, ["*.dox", "*.pdf", "*.dot", "*.png"])
    return files


def IMPModuleDoc(env, files=None, authors=None,
                 brief=None, overview=None,
                 publications=None,
                 license="standard"):
    if files:
        print >> sys.stderr, "Module docs are specified through the overview.dox file, not the SConscript file."


#   files= ["#/bin/imppy.sh", "#/tools/run_all_tests.py"]+\
#        [x.abspath for x in Glob("test_*.py")+ Glob("*/test_*.py")]

def IMPModuleTest(env, python_tests=[], cpp_tests=[],
                  expensive_python_tests=[],
                  expensive_cpp_tests=[],
                  plural_exceptions=[], show_exceptions=[],
                  function_name_exceptions=[],
                  value_object_exceptions=[],
                  class_name_exceptions=[],
                  spelling_exceptions=[],
                  check_standards=True):
    """Pseudo-builder to run tests for an IMP module. The single target is
       generally a simple output file, e.g. 'test.passed', while the single
       source is a Python script to run (usually run-all-tests.py).
       Right now, the assumption is made that run-abll-tests.py executes
       all files called test_*.py in the current directory and subdirectories."""
    # probably could run some test without python, but why bother
    if env["IMP_PASS"] != "RUN" or env["python"]=="no":
        return
    if python_tests != []:
        print >> sys.stderr, "WARNING explicitly listing tests is not supported anymore"
    module=_get_module_name(env)
    standards=[]
    if len(plural_exceptions+show_exceptions+ function_name_exceptions\
        +value_object_exceptions+class_name_exceptions+spelling_exceptions) > 0:
        print >> sys.stderr, "WARNING list your test standards exceptions in a file called \"standards_exceptions\". The file should contain:"
        print >> sys.stderr, "plural_exceptions=%s"%str(plural_exceptions)
        print >> sys.stderr, "show_exceptions=%s"%str(show_exceptions)
        print >> sys.stderr, "function_name_exceptions=%s"%str(function_name_exceptions)
        print >> sys.stderr, "value_object_exceptions=%s"%str(function_name_exceptions)
        print >> sys.stderr, "class_name_exceptions=%s"%str(class_name_exceptions)
        print >> sys.stderr, "spelling_exceptions=%s"%str(class_name_exceptions)
        standards.append(_standards.add(env, plural_exceptions=plural_exceptions,
                                 show_exceptions=show_exceptions,
                                 function_name_exceptions=function_name_exceptions,
                                 value_object_exceptions=value_object_exceptions,
                                 class_name_exceptions=class_name_exceptions,
                                 spelling_exceptions=spelling_exceptions))
    python_tests=stp.get_matching_source(env, ["test_*.py", "*/test_*.py"])\
        + stp.get_matching_build(env, ["test/%s/test_*.py"%module], ondisk=True)
    cpp_tests=stp.get_matching_source(env, ["test_*.cpp", "*/test_*.cpp"])
    expensive_python_tests= stp.get_matching_source(env, ["expensive_test_*.py",
                                         "*/expensive_test_*.py"])\
                    + stp.get_matching_build(env, ["test/%s/expensive_test_*.py"%module], ondisk=True)
    expensive_cpp_tests= stp.get_matching_source(env, ["expensive_test_*.cpp",
                                         "*/expensive_test_*.cpp"])
    files= [x.abspath for x in python_tests]
    expensive_files= [x.abspath for x in expensive_python_tests]
    deps=[]
    if len(cpp_tests)>0:
        #print "found cpp tests", " ".join([str(x) for x in cpp_tests])
        prgs= stb.handle_bins(env, cpp_tests,
                              stp.get_build_test_dir(env, module),
                              extra_modules=[module])
        deps.extend(prgs)
    if len(expensive_cpp_tests)>0:
        #print "found cpp tests", " ".join([str(x) for x in cpp_tests])
        prgs= _make_programs(env, Dir("#/build/test"), expensive_cpp_tests, prefix=module)
        deps.extend(prgs)
    tests = scons_tools.test.add_tests(env, source=files,
                                       expensive_source=expensive_files,
                                       dependencies=deps,
                                       type='module unit test')

def _get_updated_cxxflags(old, extra, removed):
    return [r for r in old if r not in removed]+extra


def split(string):
    return [x for x in string.split(":") if x != ""]

def IMPModuleBuild(env, version=None, required_modules=[],
                   lib_only_required_modules=[],
                   optional_modules=[],
                   lib_only_optional_modules=[],
                   optional_dependencies=[], config_macros=[],
                   module_libname=None,
                   module_pylibname=None,
                   module_include_path=None, module_preproc=None,
                   module_namespace=None, module_nicename=None,
                   required_dependencies=[],
                   alias_name=None,
                   extra_cxxflags=[], removed_cxxflags=[],
                   cppdefines=[], cpppath=[], python_docs=False,
                   local_module=False,
                   standards=True):
    if len(required_modules) >0 or len(lib_only_required_modules) >0:
        print >> sys.stderr, "You should use the \"description\" file to describe a modules dependencies instead of the SConscript (and remove the variables from the SConscript). One has been created."
        file=open(scons_tools.paths.get_input_path(env, "description"), "w")
        file.write("required_modules=\""+":".join(required_modules)+"\"\n")
        if lib_only_required_modules:
            file.write("lib_only_required_modules=\""+":".join(lib_only_required_modules)+"\"\n")
        if optional_modules:
            file.write("optional_modules=\""+":".join(r(optional_modules)+"\"\n"))
        if lib_only_optional_modules:
            file.write("lib_only_optional_modules="+":".join(lib_only_optional_modules)+"\"\n")
        file.write("required_dependencies=\""+":".join(required_dependencies)+"\"\n")
        file.write("optional_dependencies=\""+":".join(optional_dependencies)+"\"\n")
    else:
        required_modules=""
        lib_only_required_modules=""
        optional_modules=""
        lib_only_optional_modules=""
        required_dependencies=""
        optional_dependencies=""
        exec open(scons_tools.paths.get_input_path(env, "description"), "r").read()
        required_modules=split(required_modules)
        lib_only_required_modules=split(lib_only_required_modules)
        optional_modules=split(optional_modules)
        lib_only_optional_modules=split(lib_only_optional_modules)
        required_dependencies=split(required_dependencies)
        optional_dependencies=split(optional_dependencies)

    if env.GetOption('help'):
        return
    dta= scons_tools.data.get(env)
    module=Dir('.').abspath.split('/')[-1]
    if module != module.lower() or module.find("-") != -1:
        scons_tools.utility.report_error("Module names can only have lower case characters and numbers")
    if module_libname is None:
        module_libname="imp_"+module
    if module_pylibname is None:
        module_pylibname="_IMP_"+module
    if module_include_path is None:
        module_include_path="IMP/"+module
    if module_namespace is None:
        module_namespace="IMP::"+module
    if module_preproc is None:
        module_preproc=module_namespace.replace("::","_").upper()
    if module_nicename is None:
        module_nicename= "IMP."+module
    if alias_name is None:
        alias_name=module
    if python_docs:
        env.Append(IMP_PYTHON_DOCS=[module])
    optm=optional_modules+lib_only_optional_modules
    optd=optional_dependencies
    reqd=required_dependencies
    reqm=required_modules+lib_only_required_modules
    all_sconscripts=stp.get_sconscripts(env, ['data', 'examples'])
    nenv = scons_tools.utility.configure_module(env,
                                                module, alias_name,
                                                module_libname,
                                                version,
                                                required_modules=reqm,
                                                optional_dependencies=optd,
                                                optional_modules=optm,
                                                required_dependencies= reqd)
    if not nenv:
        return
    preclone=env

    env = nenv
    vars={'module_include_path':module_include_path,
          'module':module,
          'PREPROC':module_preproc,
          'EXPORT':module_preproc.replace("_", ""),
          'namespace':module_namespace,
          'module_libname':module_libname,
          'module_pylibname':module_pylibname,
          'module_nicename':module_nicename,
          'module_alias':alias_name}
    env['IMP_MODULE_VARS']=vars


    build_config=[]
    if removed_cxxflags or extra_cxxflags:
        env.Replace(IMP_SHLIB_CXXFLAGS=_get_updated_cxxflags(env["IMP_SHLIB_CXXFLAGS"], extra_cxxflags, removed_cxxflags))
        env.Replace(IMP_ARLIB_CXXFLAGS=_get_updated_cxxflags(env["IMP_ARLIB_CXXFLAGS"], extra_cxxflags, removed_cxxflags))
        env.Replace(IMP_BIN_CXXFLAGS=_get_updated_cxxflags(env["IMP_BIN_CXXFLAGS"], extra_cxxflags, removed_cxxflags))
        env.Replace(IMP_PYTHON_CXXFLAGS=_get_updated_cxxflags(env["IMP_PYTHON_CXXFLAGS"], extra_cxxflags, removed_cxxflags))
    if cppdefines:
        env.Append(CPPDEFINES=cppdefines)
    if cpppath:
        env.Append(CPPPATH=cpppath)
    #if len(found_optional_modules + found_optional_dependencies)>0:
    #    print "  (using " +", ".join(found_optional_modules + found_optional_dependencies) +")"
    real_config_macros=config_macros[:]

    #print "config", module, real_config_macros
    env['IMP_MODULE_CONFIG']=real_config_macros
    if env['IMP_PASS']=="BUILD":
        # must be before we recurse
        _config_h.build(env, config_macros, dta.modules[module])
    for s in all_sconscripts:
        env.SConscript(s, exports='env')

    if env['IMP_PASS']=="BUILD":

        dta.add_to_alias("all", _get_module_alias(env))
        # needed for data
        for m in _get_module_modules(env):
            env.Requires(dta.get_alias(_get_module_alias(env)),
                         dta.get_alias(dta.modules[m].alias))

        if standards:
            root=Dir(".").abspath
            if env.get('repository', None):
                old=Dir("#").abspath
            #print old, root, env['repository']
                root=root.replace(old, Dir(Dir("#").abspath+"/"+env['repository']).abspath)
                scons_tools.standards.add(env, [root+"/"+x for x in ["include/*.h",
                                                                     "include/internal/*.h",
                                                                     "src/*.cpp",
                                                                     "src/internal/*.cpp",
                                                                     "test/*.py",
                                                                     "bin/*.cpp"]])
    return env
