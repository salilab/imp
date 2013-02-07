"""Tools and Builders for IMP modules. See `IMPModule` for more information."""

import os.path
import sys
import scons_tools.pyscanner
import _swig
import _header
import _link_test
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
import scons_tools.build_tools.tools

from SCons.Script import Builder, File, Action, Glob, Return, Dir, Move, Copy, Scanner
from SCons.Scanner import C as CScanner

def _get_module_name(env):
    return scons_tools.environment.get_current_name(env)

def _get_module_alias(env):
    name= _get_module_name(env)
    if name=="test":
        return env.Alias("test_module")
    else:
        return env.Alias(name)



def IMPModuleLib(envi, files=[]):
    """Build, and optionally also install, an IMP module's C++
       shared library. This is only available from within an environment
       created by `IMPSharedLibraryEnvironment`."""
    if envi["IMP_PASS"] == "BUILD" and files != []:
        print >> sys.stderr, "WARNING, specifying sources is no longer supported"

def _lib(env):
    module = scons_tools.environment.get_current_name(env)
    build=[]
    sources=scons_tools.paths.get_matching_source(env, ["src/*.cpp",
                                                         "src/internal/*.cpp"])
    build_sources=scons_tools.paths.get_matching_build_files(env,
                                                             ["src/"+module+"/*.cpp"],
        ondisk=True)
    if env['percppcompilation']=="yes"\
           or module in env['percppcompilation'].split(":"):
        allf=sources\
            + build_sources
        if env['build']=="debug" and env['linktest']:
            link0=env.IMPModuleLinkTest(target=[stp.get_build_source_file(env,
                                                                           'link_0.cpp',
                                                                           module)],
                                          source=[])
            link1=env.IMPModuleLinkTest(target=[stp.get_build_source_file(env,
                                                                           'link_1.cpp',
                                                                            module)],
                                          source=[])
            allf= allf+link0+link1
    else:
        allcpp=File("#/build/src/%s_all.cpp"%module)
        # scons doesn't get the dependencies right since it can check all.cpp before the
        # file exists
        allf= [allcpp]+build_sources
        if env['build']=="debug" and env['linktest']:
            link1=env.IMPModuleLinkTest(target=[stp.get_build_source_file(env,
                                                                           'link.cpp',
                                                                           module)],
                                          source=[])
            allf= allf+link1
    if env['IMP_BUILD_STATIC']:
        env= scons_tools.envronment.get_staticlib_environment(env)
        sl= env.StaticLibrary('#/build/lib/imp_%s' % module,
                              allf)
        scons_tools.data.get(env).add_to_alias(_get_module_alias(env), sl[0])
    if env['IMP_BUILD_DYNAMIC']:
        env = scons_tools.environment.get_sharedlib_environment(env, '%s_EXPORTS' % module.upper(),
                                    cplusplus=True)
        sl=env.SharedLibrary('#/build/lib/imp_%s' % module,
                                       allf )
        scons_tools.data.get(env).add_to_alias(_get_module_alias(env), sl[0])
        scons_tools.utility.postprocess_lib(env, sl)


def IMPModuleInclude(env, files):
    """Install the given header files, plus any auto-generated files for this
       IMP module."""
    pass

def IMPModuleData(env, files):
    """Install the given data files for this IMP module."""
    pass


def IMPModuleExamples(env, example_files=None, data_files=None,
                      example_modules=[]):
    pass
def _examples(env):
    module= _get_module_name(env)
    example_pyfiles=stp.get_matching_source(env, ["examples/*.py"])+stp.get_matching_build(env, ["test/%s/cpp_examples_test.py"%module])
    example_cppfiles=stp.get_matching_source(env, ["examples/*.cpp"])
    runable=[File(x) for x in example_pyfiles
             if str(x).find("fragment")==-1]
    deps=[]
    if len(example_cppfiles) > 0:
        prgs= stb.handle_bins(env, example_cppfiles,
                                  stp.get_build_test_dir(env, module),
                                  # link in all modules so as not to have to bother with dependencies
            extra_modules=[module])
        deps.extend(prgs)
    if len(runable)>0:
        tests = scons_tools.test.add_tests(env,
                                           source=runable,
                                           dependencies=deps,
            type='example')

def IMPModuleBin(env, files):
    pass

def _bins(env):
    module= _get_module_name(env)
    files=stp.get_matching_source(env, ["bin/*.cpp", "bin/*.py"])
    prgs=stb.handle_bins(env, files,
                         stp.get_build_bin_dir(env, _get_module_name(env)),
                         extra_modules=[_get_module_name(env)])
    scons_tools.data.get(env).add_to_alias(_get_module_alias(env), prgs)

def IMPModuleBenchmark(env, files):
    pass

def _benchmarks(env):
    # don't disable benchmark by checking for it before it is sourced
    # compatibility and base must not have benchmarks
    files=stp.get_matching_source(env, ["benchmarks/*.cpp", "benchmarks/*.py"])
    prgs, bmarks=stb.handle_benchmarks(env, files,
                                       stp.get_build_benchmark_dir(env, _get_module_name(env)),
                                       extra_modules=[_get_module_name(env)])
    module= _get_module_name(env)
    scons_tools.data.get(env).add_to_alias(module+"-benchmarks", bmarks)
    scons_tools.data.get(env).add_to_alias(_get_module_alias(env), prgs)

def IMPModulePython(env, swigfiles=[], pythonfiles=[]):
    pass

def _pylib(env):
    """Build and install an IMP module's Python extension and the associated
       wrapper file from a SWIG interface file. This is only available from
       within an environment created by `IMPPythonExtensionEnvironment`."""
    module =_get_module_name(env)
    data=scons_tools.data.get(env)
    penv = scons_tools.environment.get_pyext_environment(env, module.upper(),
                                                         cplusplus=True,
                                                         extra_modules=[module])
    #penv.Decider('timestamp-match')
    produced=File("#/build/lib/IMP/%s/__init__.py"%module)
    cpp=stp.get_build_swig_source_file(penv,
                                    "wrap.cpp", module)
    h=stp.get_build_swig_source_file(penv,
                                  "wrap.h", module)
    swigr=penv.IMPModuleSWIG(target=[produced,
                                     cpp, h],
                             source=[ env.Value(module),
                                      env.Value(env.get("swigprogram", "swig")),
                                      env.Value(env["swigpath"]),
                                      env.Value(env["includepath"]),
                                      File("#/build/swig/IMP_%s.i"%module)])
    lpenv= scons_tools.bug_fixes.clone_env(penv)
    lpenv.Append(CPPDEFINES=["IMP_SWIG"])
    buildlib = lpenv.LoadableModule("#/build/lib/_IMP_%s"%module,
                                    [cpp]) #SCANNERS=scanners
    data.add_to_alias(_get_module_alias(env), buildlib[0])
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
    if len(plural_exceptions+show_exceptions+ function_name_exceptions\
        +value_object_exceptions+class_name_exceptions+spelling_exceptions) > 0:
        print >> sys.stderr, "WARNING list your test standards_exceptions in a file called \"test/standards_exceptions\". The file should contain:"
        print >> sys.stderr, "plural_exceptions=%s"%str(plural_exceptions)
        print >> sys.stderr, "show_exceptions=%s"%str(show_exceptions)
        print >> sys.stderr, "function_name_exceptions=%s"%str(function_name_exceptions)
        print >> sys.stderr, "value_object_exceptions=%s"%str(function_name_exceptions)
        print >> sys.stderr, "class_name_exceptions=%s"%str(class_name_exceptions)
        print >> sys.stderr, "spelling_exceptions=%s"%str(class_name_exceptions)

def _tests(env):
    module=_get_module_name(env)
    python_tests=stp.get_matching_source(env, ["test/test_*.py", "test/*/test_*.py"])\
        + stp.get_matching_build(env, ["test/%s/test_*.py"%module], ondisk=True)
    cpp_tests=stp.get_matching_source(env, ["test/test_*.cpp", "test/*/test_*.cpp"])
    expensive_python_tests= stp.get_matching_source(env, ["test/expensive_test_*.py",
                                         "test/*/expensive_test_*.py"])\
                    + stp.get_matching_build(env, ["test/%s/expensive_test_*.py"%module], ondisk=True)
    expensive_cpp_tests= stp.get_matching_source(env, ["test/expensive_test_*.cpp",
                                         "test/*/expensive_test_*.cpp"])
    files= [File(x).abspath for x in python_tests]
    expensive_files= [File(x).abspath for x in expensive_python_tests]
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


def _check(file, name, context):
    if context.env['endian'] == 'auto':
        context.Message("Checking compiler %s... "%name.replace("_", " "))
        text = File(file).get_text_contents()
        #print text
        ret = context.TryRun(text, ".cpp")
        if ret[0] == 0:
            context.Result("no")
            return "0"
        else:
            context.Result("yes")
            return "1"

def _configure_check(env, name, file):
    tenv= scons_tools.environment.get_test_environment(env)
    import functools
    custom_tests = {'Check'+name:functools.partial(_check, file, name)}
    conf = tenv.Configure(custom_tests=custom_tests)
    #if not env.GetOption('clean') and not env.GetOption('help'):
    env['IMP_'+name.upper()]=eval("conf.Check"+name+"()")
    env.Append(IMP_CONFIGURATION=[name+"='"+env['IMP_'+name.upper()]+"'"])
    #else:
    #    env['IMP_ENDIAN']="not"
    conf.Finish()

def _do_configure(env, module, config_macros):
    # get around silly python defualt arg behavior
    #oconfig_macros=[x for x in config_macros]
    #config_macros=oconfig_macros
    for path in scons_tools.paths.get_matching_source(env, ["compiler/*.cpp"]):
        name= path[path.rfind("/")+1:-4]
        _configure_check(env, name, path)
        config_macros.append(["IMP_COMPILER_%s"%name.upper(), env['IMP_'+name.upper()]])
    for p in scons_tools.paths.get_matching_source(env, ["dependency/*.description"]):
        vars= scons_tools.build_tools.tools.get_dependency_description(p)
        scons_tools.dependency.add_external_library(env, vars["name"],
                                                    vars["libraries"],
                                                    header=vars["headers"],
                                                    extra_libs=vars["extra_libraries"],
                                                    build_script=vars["build_script"],
            body=vars["body"])
    sources=[]
    cleaned_macros=[]
    for m in config_macros:
        if type(m)==list:
            cleaned_macros.append(m[0]+"="+str(m[1]))
        else:
            cleaned_macros.append(m)
    sources.append(File("#/scons_tools/build_tools/setup_module.py"))
    sources.append(env.Value("\"--source="+scons_tools.paths.get_source_root(env)+"\""))
    sources.append(env.Value("\"--name="+module+"\""))
    sources.append(env.Value("\"--defines="+":".join(cleaned_macros)+"\""))
    sources.append(env.Value("\"--datapath="+env.get("datapath", "")+"\""))
    cmd=" ".join(["cd", Dir("#/build").abspath, ";"]+[str(x) for x in sources])
    # scons doesn't get the dependencies right no matter what I do, so just write it
    # every time
    env.Execute(cmd)


def IMPModuleBuild(env, version=None, required_modules=[],
                   lib_only_required_modules=[],
                   optional_modules=[],
                   lib_only_optional_modules=[],
                   optional_dependencies=[], config_macros=[],
                   required_dependencies=[],
                   extra_cxxflags=[], removed_cxxflags=[],
                   cpppath=[], python_docs=False,
                   standards=True):
    module=Dir('.').abspath.split('/')[-1]

    if env['IMP_PASS']== 'CONFIGURE':
        _do_configure(env, module, config_macros)

        if len(required_modules) >0 or len(lib_only_required_modules) >0:
            print >> sys.stderr, "You must use the \"description\" file to describe a modules dependencies instead of the SConscript (and remove the variables from the SConscript). One has been created."
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
        return

    if env.GetOption('help'):
        return
    info = scons_tools.build_tools.tools.get_module_info(module, env.get("datapath", ""),
                                                         Dir("#/build").abspath)
    if not info["ok"]:
        return

    #if len(found_optional_modules + found_optional_dependencies)>0:
    #    print "  (using " +", ".join(found_optional_modules + found_optional_dependencies) +")"
    all_sconscripts=stp.get_sconscripts(env, ['data', 'examples'])
    for s in all_sconscripts:
        env.SConscript(s, exports='env')

    env = scons_tools.environment.get_named_environment(env, module,
                                                        info["modules"],
                                                  info["dependencies"])

    if env['IMP_PASS']=="BUILD":

        if removed_cxxflags or extra_cxxflags:
            env.Replace(IMP_SHLIB_CXXFLAGS=_get_updated_cxxflags(env["IMP_SHLIB_CXXFLAGS"], extra_cxxflags, removed_cxxflags))
            env.Replace(IMP_ARLIB_CXXFLAGS=_get_updated_cxxflags(env["IMP_ARLIB_CXXFLAGS"], extra_cxxflags, removed_cxxflags))
            env.Replace(IMP_BIN_CXXFLAGS=_get_updated_cxxflags(env["IMP_BIN_CXXFLAGS"], extra_cxxflags, removed_cxxflags))
            env.Replace(IMP_PYTHON_CXXFLAGS=_get_updated_cxxflags(env["IMP_PYTHON_CXXFLAGS"], extra_cxxflags, removed_cxxflags))
        if cpppath:
            env.Append(CPPPATH=cpppath)
        _lib(env)
        _pylib(env)
        _bins(env)
        _benchmarks(env)
        scons_tools.data.get(env).add_to_alias("all", [_get_module_alias(env)])
        # needed for data
        for m in info["modules"]:
            env.Requires(_get_module_alias(env), m)
    elif env['IMP_PASS']=="RUN":
        _tests(env)
        _examples(env)
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
