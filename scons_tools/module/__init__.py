"""Tools and Builders for IMP modules. See `IMPModule` for more information."""

import os.path
import sys
import scons_tools.pyscanner
import _swig
import _header
import _version_info
import _link_test
import _standards
import _version_h
import _config_h
import _all_cpp
import scons_tools.bug_fixes
import scons_tools.run
import scons_tools.dependency
import scons_tools.doc
import scons_tools.environment
import scons_tools.examples
import scons_tools.install
import scons_tools.utility

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
    else:
        pass
    return name

def _get_module_data(env, module=None):
    if not module:
        module= _get_module_name(env)
    return scons_tools.data.get(env).modules[module]

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

def _get_module_python_modules(env):
    return _get_module_data(env).python_modules

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
def _get_module_links(env):
    return env['IMP_MDLE_LINKS']


def _get_module_variables(env):
    """Make a map which can be used for all string substitutions"""
    return env['IMP_MODULE_VARS']


def IMPModuleLib(envi, files):
    """Build, and optionally also install, an IMP module's C++
       shared library. This is only available from within an environment
       created by `IMPSharedLibraryEnvironment`."""
    vars= _get_module_variables(envi)
    module = _get_module_name(envi)
    module_libname =_get_module_variables(envi)['module_libname']
    version= _get_module_version(envi)
    data= scons_tools.data.get(envi).modules[_get_module_name(envi)]
    prefix=vars['module_libname']
    if prefix=="imp":
        prefix="imp_kernel"
    config= envi.IMPModuleConfigCPP(target=["#/build/src/"+prefix+"_config.cpp"],
                                   source=[envi.Value(version),
                                           envi.Value(envi.subst(envi['datadir'])),
                                           envi.Value(envi.subst(os.path.join(envi['docdir'], "examples")))])
    #env.AlwaysBuild(version)
    build=[]
    if envi['percppcompilation']=="yes"\
           or module in envi['percppcompilation'].split(":"):
        allf=files+config
        if envi['build']=="debug" and envi['linktest']:
            link0=envi.IMPModuleLinkTest(target=['#/build/src/%(module_libname)s_link_0.cpp'%vars],
                                          source=[])
            link1=envi.IMPModuleLinkTest(target=['#/build/src/%(module_libname)s_link_1.cpp'%vars],
                                          source=[])
            allf= allf+link0+link1
    else:
        allf= [_all_cpp.get(envi, list(files))]+config
        if envi['build']=="debug" and envi['linktest']:
            link1=envi.IMPModuleLinkTest(target=['#/build/src/%(module_libname)s_link.cpp'%vars],
                                          source=[])
            allf= allf+link1
    if envi['IMP_BUILD_STATIC']:
        env= scons_tools.environment.get_staticlib_environment(envi)
        sl= env.StaticLibrary('#/build/lib/%s' % module_libname,
                              allf)
        data.build.append(sl[0])
        scons_tools.install.install(env, "libdir", sl[0])
    if envi['IMP_BUILD_DYNAMIC']:
        env = scons_tools.environment.get_sharedlib_environment(envi, '%(EXPORT)s_EXPORTS' % vars,
                                    cplusplus=True)
        sl=env.SharedLibrary('#/build/lib/%s' % module_libname,
                                       allf )
        data.build.append(sl[0])
        scons_tools.utility.postprocess_lib(env, sl)
        scons_tools.install.install(env, "libdir", sl[0])


def IMPModuleInclude(env, files):
    """Install the given header files, plus any auto-generated files for this
       IMP module."""
    vars=_get_module_variables(env)

    # Generate config header and SWIG equivalent
    from scons_tools.install import get_build_path as gbp
    version=env.IMPModuleVersionH(target\
                               =[gbp(env,
                                     'includedir/%s/currentfile_version.h'%vars['module_include_path'])],
                               source=[env.Value(_get_module_version(env))])
    scons_tools.install.install(env, "includedir/%s"%vars['module_include_path'],
                                              version[0])
    data= scons_tools.data.get(env)
    deps= _get_module_dependencies(env)
    signature=_get_module_unfound_dependencies(env)\
               + deps\
               + [data.dependencies[x].version for x in deps]
    config=env.IMPModuleConfigH(target\
                               =[gbp(env,
                                     'includedir/%s/currentfile_config.h'%vars['module_include_path'])],
                               source=[env.Value(env['IMP_MODULE_CONFIG']),
                                       env.Value(signature)])
    scons_tools.install.install(env, "includedir/%s"%vars['module_include_path'],
                                              config[0])
    scons_tools.install.install_hierarchy(env, "includedir/%s"%vars['module_include_path'],
                                          "include",
                                         list(files))
    if _get_module_name(env)=='kernel':
        hn= scons_tools.install.get_build_path(env,
                                               "includedir/IMP.h")

    else:
        hn=scons_tools.install.get_build_path(env,
                                              "includedir/%s/../%s.h"%(vars['module_include_path'], vars['module']))
    header=_header.build_header(env, hn,
                                list(files))
    scons_tools.install.install(env, "includedir/%s/.."%vars['module_include_path'], header[0])

def IMPModuleData(env, files):
    """Install the given data files for this IMP module."""
    data=scons_tools.data.get(env).modules[_get_module_name(env)]
    for f in files:
        (build, install)=scons_tools.install.install(env, "datadir/currentdir/", f)
        data.build.append(build)


def IMPModuleExamples(env, example_files, data_files):
    #print "Examples called with",[str(x) for x in example_files],\
    #    [str(x) for x in data_files]
    example_files= [File(x) for x in example_files]
    #for f in example_files:
    #    print f
    #    print f.abspath
    #    open(f.abspath, 'r')
    (build, install)=scons_tools.install.install_hierarchy(env, "docdir/examples/currentdir",
                                                           "examples",
                                                           example_files+data_files)
    test= scons_tools.test.add_test(env,
                                    source=[x for x in example_files
                                     if str(x).endswith(".py") \
                                     and str(x).find("fragment")==-1],
                                    type='example')
    env.Requires(test, build)
    scons_tools.data.get(env).add_to_alias(_get_module_name(env), build)
    scons_tools.data.get(env).add_to_alias(_get_module_name(env)+"-install", install)
    seen=[]
    for e in example_files:
        if str(e).endswith(".py"):
            for o in example_files:
                if str(o) == scons_tools.utility.get_without_extension(str(e))+".readme":
                    seen.append(o)
                    break
            scons_tools.examples.add_python_example(env, e, o.get_contents())
    links=[]
    fexample_files=[x for x in example_files if x not in seen]
    split= scons_tools.utility.get_split_into_directories(fexample_files)
    for k in split.keys():
        if len(k)>0:
            name =_get_module_name(env)+ " examples: "+k
            pre=k+'/'
        else:
            name =_get_module_name(env)+ " examples"
            pre=""
        name=scons_tools.utility.get_display_from_name(name)
        l= scons_tools.examples.add_page(env, name,
                                      [pre+x for x in split[k]])
        links.append(l)
    _set_module_links(env, links)

def _make_programs(envi, files):
    env= scons_tools.environment.get_bin_environment(envi,
                                                     extra_modules=[_get_module_name(envi)])
    ret=[]
    for f in files:
        if str(f).endswith(".cpp"):
            ret.append(env.Program(f))
        elif str(f).endswith(".py"):
            dest=Dir('#/modules/'+scons_tools.environment.get_current_name(env)+'/bin/')
            #print "handling", f.abspath, dest.path
            nm=f.path[f.path.rfind("/")+1:]
            #print dest.path, f.path, nm
            ret.append(scons_tools.install.install(env, dest.path, nm))
    return ret

def IMPModuleBin(env, files):
    prgs=_make_programs(env, files)
    scons_tools.data.get(env).add_to_alias(_get_module_name(env), prgs)


def _fake_scanner_cpp(node, env, path):
    if node.abspath.endswith(".h") or node.abspath.endswith(".cpp"):
        print "fake scanning", node.abspath
    if _get_module_name(env) == 'kernel':
        return [File("#/build/include/IMP.h")]
    else:
        return ([File("#/build/include/IMP/"+_get_module_name(env)+".h")]\
               + [File("#/build/include/IMP/"+x+".h") for x in _get_module_modules(env)]+ [File("#/build/include/IMP.h")]).sorted()

def _filtered_h(node, env, path):
    #print "filtered scanning", node.abspath
    if node.abspath.endswith(".h") or node.abspath.endswith(".cpp"):
        print "fake scanning", node.abspath
    if  node.abspath.find('build') != -1:
        return []
    else:
        return CScanner()(node, env, path)

def IMPModulePython(env, swigfiles=[], pythonfiles=[]):
    """Build and install an IMP module's Python extension and the associated
       wrapper file from a SWIG interface file. This is only available from
       within an environment created by `IMPPythonExtensionEnvironment`."""
    module =_get_module_name(env)
    vars=_get_module_variables(env)
    data=scons_tools.data.get(env).modules[_get_module_name(env)]
    alldata= scons_tools.data.get(env).modules
    penv = scons_tools.environment.get_pyext_environment(env, module.upper(),
                                                         cplusplus=True,
                                                         extra_modules=[_get_module_name(env)])
    #penv.Decider('timestamp-match')
    """scanners=[Scanner(function= _fake_scanner_cpp, skeys=['.cpp']),
              Scanner(function=_filtered_h, skeys=['.h']),
              #Scanner(function= _fake_scanner_i, skeys=['.i']),
              Scanner(function=_swig._null_scanner, skeys=[".cpp-in", ".h-in", ".i-in"])]
    penv.Replace(SCANNERS=scanners)"""
    from scons_tools.install import get_build_path as gbp
    versions=[]
    for m in _get_module_python_modules(env):
        versions.append(env.Value(m))
        versions.append(env.Value(alldata[m].version))
    vc= _swig.VersionCheck(penv, target=[gbp(penv, "libdir/%s/_version_check.py"%vars['module_include_path'])],
                           source=[env.Value(_get_module_version(env))]+versions)
    data.build.append(vc[0])
    scons_tools.install.install(penv, 'pythondir/%s'%vars['module_include_path'], vc[0])

    prefix=vars['module_pylibname'][1:]
    if prefix=="IMP":
        prefix="IMP_kernel"
    swigfile= \
       penv.IMPModuleSWIGPreface(target=[gbp(penv, "swigdir/"+prefix+".i")],
                                 source=[File("swig.i-in"),
                                         env.Value(_get_module_python_modules(env)),
                                         env.Value(" ".join(_get_module_dependencies(env))),
                                  env.Value(" ".join(_get_module_unfound_dependencies(env)))])
    scons_tools.install.install(penv, "datadir/swig", swigfile)
    vc
    for i in swigfiles:
        if str(i).endswith('.i'):
            scons_tools.install.install(env,"swigdir", i)
    produced=gbp(penv, "srcdir/%s.py"%vars['module_include_path'].replace("/", "."))
    version=_get_module_version(penv)
    cppin=gbp(penv, 'srcdir/'+prefix+'_wrap.cpp-in')
    hin=gbp(penv, 'srcdir/'+prefix+'_wrap.h-in')
    swigr=penv.IMPModuleSWIG(target=[produced,
                                     cppin, hin],
                             source=[swigfile])
    #print "Moving", produced.path, "to", dest.path
    (build, install)= scons_tools.install.install_as(penv,
                                                     'pythondir/%s/__init__.py'\
                                                     %vars['module_include_path'],
                                   produced)
    data.build.append(build)
    cppf=gbp(penv, 'srcdir/'+prefix+'_wrap.cpp')
    hf=gbp(penv, 'srcdir/'+prefix+'_wrap.h')
    patched=penv.IMPModulePatchSWIG(target=[cppf],
                               source=[cppin])
    hpatched=penv.IMPModulePatchSWIG(target=[hf],
                       source=[hin])
    penv.Requires(patched, hpatched)
    lpenv= scons_tools.bug_fixes.clone_env(penv)
    buildlib = lpenv.LoadableModule(gbp(penv, 'libdir/%(module_pylibname)s' %
                                       _get_module_variables(lpenv)),
                                    patched) #SCANNERS=scanners
    data.build.append(buildlib[0])
    inst=scons_tools.install.install(penv, 'pyextdir', buildlib[0])
    scons_tools.utility.postprocess_lib(penv, buildlib)
    (b,s)= scons_tools.install.install_hierarchy(env, 'pythondir/%s'%vars['module_include_path'],
                                         "src", pythonfiles)
    for bs in b:
        #print bs
        data.build.append(bs)

def IMPModuleGetExamples(env):
    rms= scons_tools.utility.get_matching_recursive(["*.readme"])
    ex= [x for x in scons_tools.utility.get_matching_recursive(["*.py"])
         if str(x) != "test_examples.py"]
    return rms+ex

def IMPModuleGetExampleData(env):
    ret=  scons_tools.utility.get_matching_recursive(["*.pdb", "*.mrc", "*.dat",
                                                      "*.xml", "*.em", "*.rmf",
                                                      "*.hdf5", "*.mol2"])
    return ret

def IMPModuleGetPythonTests(env):
    return scons_tools.utility.get_matching_recursive(["test_*.py"])
def IMPModuleGetCPPTests(env):
    return scons_tools.utility.get_matching_recursive(["test_*.cpp"])
def IMPModuleGetExpensivePythonTests(env):
    return scons_tools.utility.get_matching_recursive(["expensive_test_*.py"])
def IMPModuleGetExpensiveCPPTests(env):
    return scons_tools.utility.get_matching_recursive(["expensive_test_*.cpp"])


def IMPModuleGetHeaders(env):
    raw_files=scons_tools.utility.get_matching_recursive(["*.h"])
    files=[]
    for f in raw_files:
        s= str(f)
        #print s
        fname= os.path.split(s)[1]
        if fname.startswith("."):
            continue
        if s=="%(module)s_config.h"%_get_module_variables(env):
            continue
        files.append(f)
    return files

def IMPModuleGetSwigFiles(env):
    vars=_get_module_variables(env)
    prefix=vars['module_pylibname'][1:]
    files=scons_tools.utility.get_matching([prefix+"_*.i"])
    return files

def IMPModuleGetPython(env):
    files=scons_tools.utility.get_matching(["src/*.py", "src/*/*.py",
                                            "src/*/*/*.py", "src/*/*/*/*.py",
                                            "src/*/*/*/*/*.py"])
    return files

def IMPModuleGetSources(env):
    raw_files=scons_tools.utility.get_matching_recursive(["*.cpp"])
    files=[]
    for f in raw_files:
        s= str(f)
        #print s
        fname= os.path.split(s)[1]
        if fname.startswith("."):
            continue
        if s== "internal/link_0.cpp":
            continue
        if s== "internal/link_1.cpp":
            continue
        if s=="%(module)s_config.cpp"%_get_module_variables(env):
            continue
        files.append(f)
    return files

def IMPModuleGetData(env):
    raw_files=scons_tools.utility.get_matching_recursive(["*"])
    files=[]
    for f in [os.path.split(str(x))[1] for x in raw_files]:
        if str(f).endswith("SConscript"):
            continue
        if str(f).endswith(".old"):
            continue
        if str(f).startswith("."):
            continue
        if str(f).endswith("~"):
            continue
        files.append(f)
    return files

def IMPModuleGetBins(env):
    raw_files= scons_tools.utility.get_matching(["*.cpp", "*.py"])
    return raw_files

def IMPModuleGetDocs(env):
    files=scons_tools.utility.get_matching(["*.dox", "*.pdf", "*.dot", "*.png"])
    return files


def IMPModuleDoc(env, files, authors,
                 brief, overview,
                 publications=None,
                 license="standard"):
    docdir=env['docdir']+"/"+_get_module_variables(env)['module_include_path']
    links= _get_module_links(env)
    if len(links) > 0:
        overview+= '\n\nExamples:\n'
        for l in links:
            overview+=' - ' +l +'\n'
        # defined in doc so all examples have been seen
        overview += ' - \\ref '+_get_module_name(env)\
                    +'_all_example_index "All examples using IMP.'+_get_module_name(env)+'"\n'
    scons_tools.doc.add_doc_page(env,
                                 "\\namespace "\
                                 +_get_module_variables(env)['namespace']\
                                 +'\n\\brief '+brief,
                                 authors,_get_module_version(env),
                                 brief, overview, publications, license)
    scons_tools.doc.add_doc_files(env, files)


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
    files= [x.abspath for x in python_tests]
    expensive_files= [x.abspath for x in expensive_python_tests]
    if len(cpp_tests)>0:
        #print "found cpp tests", " ".join([str(x) for x in cpp_tests])
        prgs= _make_programs(env, cpp_tests)
        #print [x[0].abspath for x in prgs]
        cpptest= env.IMPModuleCPPTest(target="cpp_test_programs.py",
                                       source= prgs)
        files.append(cpptest)
    if len(expensive_cpp_tests)>0:
        #print "found cpp tests", " ".join([str(x) for x in cpp_tests])
        prgs= _make_programs(env, expensive_cpp_tests)
        #print [x[0].abspath for x in prgs]
        cpptest= env.IMPModuleCPPTest(target="cpp_test_programs.py",
                                       source= prgs)
        expensive_files.append(cpptest)
    if check_standards:
        standards=_standards.add(env, plural_exceptions=plural_exceptions,
                                 show_exceptions=show_exceptions,
                                 function_name_exceptions=function_name_exceptions,
                                 value_object_exceptions=value_object_exceptions,
                                 class_name_exceptions=class_name_exceptions,
                                 spelling_exceptions=spelling_exceptions)
        found=False
        for f in files:
            if str(f).endswith("test_standards.py"):
                found=f
        if found:
            files.remove(found)
        files.append(standards)
    test = scons_tools.test.add_test(env, source=files,
                                     expensive_source=expensive_files,
                                     type='unit test')

def IMPModuleBuild(env, version, required_modules=[],
                   lib_only_required_modules=[],
                   optional_modules=[],
                   lib_only_optional_modules=[],
                   optional_dependencies=[], config_macros=[],
                   module=None, module_libname=None,
                   module_pylibname=None,
                   module_include_path=None, module_preproc=None,
                   module_namespace=None, module_nicename=None,
                   required_dependencies=[],
                   cxxflags=[], cppdefines=[], python_docs=False,
                   local_module=False, python=True, data=True):
    if env.GetOption('help'):
        return

    if module is None:
        module=Dir('.').abspath.split('/')[-1]
        if module=="local":
            module=Dir('.').abspath.split('/')[-2]+"_local"
    if not module_libname and (module != module.lower() or module.find("-") != -1):
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
    if python_docs:
        env.Append(IMP_PYTHON_DOCS=[module])
    (nenv, version, found_optional_modules_out, found_optional_dependencies)\
         = scons_tools.utility.configure(env, module, "module", version,
                             required_modules=required_modules+lib_only_required_modules,
                             optional_dependencies=optional_dependencies,
                             optional_modules= optional_modules+lib_only_optional_modules,
                             required_dependencies= required_dependencies)
    found_optional_modules=[]
    found_lib_only_optional_modules=[]
    if found_optional_modules_out:
        for m in found_optional_modules_out:
            if m in optional_modules:
                found_optional_modules.append(m)
            else:
                found_lib_only_optional_modules.append(m)
    if nenv:
        scons_tools.data.get(env).add_module(module,
                                 modules= required_modules+found_optional_modules\
                                     +lib_only_required_modules+found_lib_only_optional_modules,
                                             unfound_modules=[x for x in optional_modules+lib_only_optional_modules if x not in
                                              found_optional_modules],
                                 python_modules=required_modules+found_optional_modules,
                                 dependencies=[x for x in found_optional_dependencies if x in optional_dependencies]\
                                     +required_dependencies,
                                             libname= module_libname,
                                 unfound_dependencies=[x for x in optional_dependencies if not x in found_optional_dependencies], version=version)
    else:
        scons_tools.data.get(env).add_module(module, ok=False)
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
          'module_nicename':module_nicename}
    env['IMP_MODULE_VARS']=vars


    build_config=[]
    if cxxflags:
        env.Replace(CXXFLAGS=cxxflags)
    if cppdefines:
        env.Append(CPPDEFINES=cppdefines)

    #if len(found_optional_modules + found_optional_dependencies)>0:
    #    print "  (using " +", ".join(found_optional_modules + found_optional_dependencies) +")"
    real_config_macros=config_macros[:]

    #print "config", module, real_config_macros
    env['IMP_MODULE_CONFIG']=real_config_macros
    env.SConscript('examples/SConscript', exports='env')
    env.SConscript('doc/SConscript', exports='env')
    if data:
        env.SConscript('data/SConscript', exports='env')
        # evil hack for now
        env['MODULE_HAS_DATA']=True
    else:
        env['MODULE_HAS_DATA']=False
    env.SConscript('include/SConscript', exports='env')
    env.SConscript('src/SConscript', exports='env')
    env.SConscript('bin/SConscript', exports='env')
    if env['IMP_PROVIDE_PYTHON'] and python:
        env.SConscript('pyext/SConscript', exports='env')
        env.SConscript('test/SConscript', exports='env')
    scons_tools.data.get(env).add_to_alias("all", module)
    for m in _get_module_modules(env):
        env.Requires(scons_tools.data.get(env).get_alias(module+"-install"),
                     scons_tools.data.get(env).get_alias(m+"-install"))
        env.Requires(scons_tools.data.get(env).get_alias(module),
                     scons_tools.data.get(env).get_alias(m))
    return env
