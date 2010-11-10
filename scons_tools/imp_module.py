"""Tools and Builders for IMP modules. See `IMPModule` for more information."""

import os.path
import sys
import pyscanner
import examples
import test
import swig
import hierarchy
import version_info
import config_h
import link_test
import bug_fixes
import modeller_test
import run
import dependency
import modpage
import pch
import utility

from SCons.Script import Builder, File, Action, Glob, Return, Alias, Dir, Move, Copy, Scanner
from SCons.Scanner import C as CScanner

def get_module_name(env):
    return env['IMP_MODULE_NAME']

def _set_module_name(env, module):
    env['IMP_MODULE_NAME']=module

def get_found_modules(env, modules):
    ret=[]
    for m in modules:
        if env.get_module_ok(m):
            ret.append(m)
    return ret

def get_module_ok(env, module=None):
    if not module:
        module= get_module_name(env)
    return env.get("IMP"+module+"_ok", False)

def _set_module_ok(env, module, ok):
    env["IMP"+module+"_ok"]=ok

def get_module_modules(env, module=None):
    if not module:
        module= get_module_name(env)
    return env['IMP'+module+'_modules']

def _set_module_modules(env, module, modules):
    l=[]
    for m in modules:
        l+=[m]+ env.get_module_modules(m)
    fl=[]
    for i,m in enumerate(l):
        if not m in l[i+1:]:
            fl.append(m)
    env['IMP'+module+'_modules']=fl

def get_module_python_modules(env, module=None):
    if not module:
        module= get_module_name(env)
    return env['IMP'+module+'_python_modules']

def _set_module_python_modules(env, module, modules):
    l=[]
    for m in modules:
        l+=[m]+ env.get_module_modules(m)
    fl=[]
    for i,m in enumerate(l):
        if not m in l[i+1:]:
            fl.append(m)
    env['IMP'+module+'_python_modules']=fl

def get_module_version(env, module=None):
    if not module:
        module= get_module_name(env)
    return env['IMP'+module+'_version']

def _set_module_version(env, module, version, deps):
    if version == "SVN" and env['svn'] and env['SVNVERSION']:
        if env.get('repository'):
            rep=env['repository']
            dp= os.path.commonprefix([Dir("#/").abspath, Dir(".").abspath])
            pf=Dir(".").abspath[len(dp)+1:]
            #print pf
            reppath=Dir("#/"+rep).abspath
            path=os.path.join(reppath, pf)
        else:
            path=Dir(".").abspath
        try:
            vr= os.popen(env['SVNVERSION'] + ' ' + path).read()
            version= "SVN "+vr.split("\n")[0]
        except OSError, detail:
            print >> sys.stderr, "WARNING: Could not run svnversion: %s" % str(detail)

    if len(deps)>0:
        version=version+" with "+", ".join(deps)
    env['IMP'+module+'_version']=version

def get_module_unfound_dependencies(env, module=None):
    if not module:
        module= get_module_name(env)
    return env['IMP'+module+'_missing_dependencies']

def _set_module_unfound_dependencies(env, module, missing):
    env['IMP'+module+'_missing_dependencies']=missing

def get_module_dependencies(env, module=None):
    if not module:
        module= get_module_name(env)
    return env['IMP'+module+'_dependencies']

def _set_module_dependencies(env, module, deps):
    l=[]
    for m in env.get_module_modules(module):
        l+= env.get_module_dependencies(m)
    fl=[]
    for i,m in enumerate(l):
        if not m in l[i+1:]:
            fl.append(m)
    env['IMP'+module+'_dependencies']=deps+fl



def module_requires(env, target, source):
    """Make sure that 'module-source' is built before 'module-target'"""
    for t in target:
        env.Requires(t, [env.Alias(get_module_name(env)+"-"+source)])


def module_depends(env, target, source):
    """Make sure that 'module-source' is built before 'module-target'"""
    env.Depends(target, [env.Alias(get_module_name(env)+"-"+source)])

def module_alias(env, target, source, is_default=False):
    """Add an alias called 'module-target' which builds source"""
    name=get_module_name(env)+"-"+target
    #print "created alias", name
    a=env.Alias(name, [source])
    if is_default:
        env.Default(a)

def add_to_global_alias(env, target, source):
    """Add the module alias 'module-source' to the global alias list 'target'"""
    env.Alias(env.Alias(target), [env.Alias(get_module_name(env)+'-'+source)])


def add_to_module_alias(env, target, source):
    """Add the alias 'module-source' to the alias 'module-target'"""
    env.Alias(env.Alias(get_module_name(env)+'-'+target),
              [env.Alias(get_module_name(env)+'-'+source)])


def module_deps_requires(env, target, source, dependencies):
    """For each of the module dependency make sure that 'moduledep-source'
    is built before 'target'"""
    for d in dependencies + env.get_module_modules():
        #print str(target) + " requires " + d+'-'+source
        env.Requires(target, env.Alias(d+'-'+source))
    if get_module_name(env) != 'kernel':
        env.Requires(target, env.Alias("kernel-"+source))



def module_deps_depends(env, target, source, dependencies):
    """For each of the module dependency make sure that 'moduledep-source'
    is built before 'target'"""
    for d in dependencies +env.get_module_modules():
        #print str(target) + " requires " + d+'-'+source
        env.Depends(target, env.Alias(d+'-'+source))

def get_module_variables(env):
    """Make a map which can be used for all string substitutions"""
    return env['IMP_MODULE_VARS']


def IMPModuleLib(envi, files):
    """Build, and optionally also install, an IMP module's C++
       shared library. This is only available from within an environment
       created by `IMPSharedLibraryEnvironment`."""
    from scons_tools import get_sharedlib_environment, get_staticlib_environment
    vars= get_module_variables(envi)
    module = get_module_name(envi)
    module_suffix = get_module_variables(envi)['module_suffix']
    if envi['build']=="debug" and envi['linktest']:
        link= envi.IMPModuleLinkTest(target=['#/build/src/%(module)s_link_0.cpp'%vars, '#/build/src/%(module)s_link_1.cpp'%vars], source=[])
        files= files+link
    version= envi.get_module_version()
    config= envi.IMPModuleConfigCPP(target=["#/build/src/%(module)s_config.cpp"%vars],
                                   source=[envi.Value(version),
                                           envi.Value(envi.subst(envi['datadir'])),
                                           envi.Value(envi.subst(os.path.join(envi['docdir'], "examples")))])
    #env.AlwaysBuild(version)
    files =files+ config
    build=[]
    if envi['IMP_BUILD_STATIC']:
        env= get_staticlib_environment(envi)
        utility.add_link_flags(env, env.get_module_modules(),
                               env.get_module_dependencies())
        build.append( env.StaticLibrary('#/build/lib/imp%s' % module_suffix,
                                      list(files)))
    if envi['IMP_BUILD_DYNAMIC']:
        env = get_sharedlib_environment(envi, '%(PREPROC)s_EXPORTS' % vars,
                                    cplusplus=True)
        utility.add_link_flags(env,env.get_module_modules(),
                               env.get_module_dependencies())
        build.append(env.SharedLibrary('#/build/lib/imp%s' % module_suffix,
                                       list(files) ) )
        utility.postprocess_lib(env, build[-1])
    install=[]
    for b in build:
        install.append(envi.Install(env.GetInstallDirectory('libdir'), b) )
    utility.postprocess_lib(envi, install[-1])
    module_requires(envi, build, 'include')
    module_requires(envi, build, 'data')
    module_alias(envi, 'lib', build, True)
    add_to_global_alias(envi, 'all', 'lib')
    module_alias(envi, 'install-lib', install)
    add_to_module_alias(envi, 'install', 'install-lib')
    module_deps_requires(envi, build, 'include', [])
    module_deps_requires(envi, build, 'lib', [])
    module_deps_requires(envi, install, 'install-lib', [])
    env.Requires(build, '#/tools/imppy.sh')


def IMPModuleInclude(env, files):
    """Install the given header files, plus any auto-generated files for this
       IMP module."""
    vars=get_module_variables(env)
    includedir = env.GetInstallDirectory('includedir')

    # Generate config header and SWIG equivalent
    config=env.IMPModuleConfigH(target=['#/build/include/%(module_include_path)s/%(module)s_config.h'%vars],
    source=[env.Value(env['IMP_MODULE_CONFIG'])])
    configinstall=env.Install(includedir+"/"+vars['module_include_path'],config)
    install = hierarchy.InstallHierarchy(env, includedir+"/"+vars['module_include_path'],
                                         list(files))+[configinstall]
    build=hierarchy.InstallHierarchy(env, "#/build/include/"+vars['module_include_path'],
                                     list(files), True)+[config]
    env['IMP_MODULE_HEADERS']= [str(x) for x in files if str(x).find("internal") == -1]
    module_alias(env, 'include', build)
    add_to_global_alias(env, 'all', 'include')
    module_alias(env, 'install-include', install)
    add_to_module_alias(env, 'install', 'install-include')

def IMPModuleData(env, files):
    """Install the given data files for this IMP module."""
    vars=get_module_variables(env)
    datadir = env.GetInstallDirectory('datadir')
    if vars['module']== 'kernel':
        path=""
    else:
        path=vars['module']
    install = hierarchy.InstallDataHierarchy(env, datadir+"/"+path, files, False)
    build = hierarchy.InstallDataHierarchy(env, "#/build/data/"+path, files, True)
    module_alias(env, 'data', build)
    add_to_global_alias(env, 'all', 'data')
    module_alias(env, 'install-data', install)
    add_to_module_alias(env, 'install', 'install-data')


def IMPModuleExamples(env, example_files, data_files):
    vars=get_module_variables(env)
    #for f in files:
    #    print f.abspath
    if vars['module']== 'kernel':
        path=""
    else:
        path=vars['module']
    (dox, build, install, test)= examples.handle_example_dir(env, Dir("."), vars['module'], path, example_files,data_files)
    module_alias(env, 'examples', build)
    add_to_global_alias(env, 'all', 'examples')
    module_alias(env, 'install-examples', install)
    add_to_global_alias(env, 'doc-install', 'install-examples')
    module_alias(env, 'test-examples', test)
    module_requires(env, test, 'examples')
    add_to_global_alias(env, 'test', 'test-examples')
    module_alias(env, 'dox-examples', dox)
    add_to_global_alias(env, 'doc', 'dox-examples')
    return test

def _make_programs(envi, required_modules, install, files):
    from scons_tools import get_bin_environment
    env= get_bin_environment(envi)
    vars=get_module_variables(env)
    if env['fastlink']:
        if get_module_name(env) != "kernel":
            env.Append(LINKFLAGS=['-limp_'+get_module_name(env)])
        else:
            env.Append(LINKFLAGS=['-limp'])
    utility.add_link_flags(env, env.get_module_modules()+required_modules,
                           env.get_module_dependencies())
    if not env['fastlink']:
        env.Prepend(LIBS=['imp%(module_suffix)s' % vars])
    build=[]
    install_list=[]
    bindir = env.GetInstallDirectory('bindir')
    allprogs=[]
    for f in files:
        if str(f).endswith(".cpp"):
            prog= env.Program(f)
            allprogs.append(prog)
            cb= env.Install("#/build/bin", prog)
            build.append(cb)
            if install:
                ci= env.Install(bindir, prog)
                install_list.append(ci)
        else:
            cb= env.Install("#/build/bin", f)
            build.append(cb)
            if install:
                ci= env.Install(bindir, f)
                install_list.append(ci)
    if env['fastlink']:
        module_requires(env, build, 'lib')
        for l in required_modules:
            env.Requires(build, env.Alias(l+'-lib'))
    return (build, install_list)

def IMPModuleBin(env, files, required_modules=[], install=True):
    (build, install_list)= _make_programs(env, required_modules, install, files)
    env['IMP_MODULE_BINS']= build
    module_alias(env, 'bin', build, True)
    add_to_global_alias(env, 'all', 'bin')
    if install:
        module_alias(env, 'install-bin', install_list)
        add_to_module_alias(env, 'install', 'install-bin')
    module_requires(env, build, 'include')
    module_requires(env, build, 'lib')
    module_requires(env, build, 'data')
    module_deps_requires(env, build, 'lib', required_modules)


def _fake_scanner_cpp(node, env, path):
    print "fake cpp", node.abspath
    if get_module_name(env) == 'kernel':
        return [File("#/build/include/IMP.h")]
    else:
        return ([File("#/build/include/IMP/"+get_module_name(env)+".h")]\
               + [File("#/build/include/IMP/"+x+".h") for x in env.get_module_modules()]+ [File("#/build/include/IMP.h")]).sorted()

def _null_scanner(node, env, path):
    #print "null scanning", node.abspath
    return []
def _filtered_h(node, env, path):
    #print "filtered scanning", node.abspath
    if  node.abspath.find('build') != -1:
        return []
    else:
        return CScanner()(node, env, path)

def IMPModulePython(env, swigfiles=[], pythonfiles=[]):
    """Build and install an IMP module's Python extension and the associated
       wrapper file from a SWIG interface file. This is only available from
       within an environment created by `IMPPythonExtensionEnvironment`."""
    from scons_tools import get_pyext_environment
    module = get_module_name(env)
    vars=get_module_variables(env)
    pybuild=[]
    install=[]
    penv = get_pyext_environment(env, module.upper(), cplusplus=True)
    #penv.Decider('timestamp-match')
    scanners=[Scanner(function= _fake_scanner_cpp, skeys=['.cpp']),
              Scanner(function=_filtered_h, skeys=['.h']),
              #Scanner(function= _fake_scanner_i, skeys=['.i']),
              swig.scanner,
              Scanner(function=_null_scanner, skeys=[".cpp-in", ".h-in", ".i-in"])]
    penv.Replace(SCANNERS=scanners)
    utility.add_link_flags(penv, [get_module_name(penv)]+penv.get_module_modules(), penv.get_module_dependencies())
    swigfile= penv.IMPModuleSWIGPreface(target=[File("#/build/swig/IMP_%(module)s.i"%vars)],
                                   source=[File("swig.i-in"),
                                           env.Value(env.get_module_python_modules()),
                                           env.Value(env.get_module_version()),
                                           #python supports serialization of object, why on earth do they just convert them to strings?
                                           env.Value(" ".join(env.get_module_dependencies())),
                                           env.Value(" ".join(env.get_module_unfound_dependencies())),])
    swiglink=[]
    for i in swigfiles:
        if str(i).find('/')==-1:
            swiglink.append( env.LinkInstallAs("#/build/swig/"+str(i), i) )
    dest = File('#/build/lib/%(module_include_path)s/__init__.py' % vars)
    produced=File("#/build/src/"+vars['module_include_path'].replace("/",".")+".py")
    version=get_module_version(penv)
    swigr=penv.IMPModuleSWIG(target=[produced, '#/build/src/%(module)s_wrap.cpp-in'%vars,
                               '#/build/src/%(module)s_wrap.h-in'%vars],
                       source=[swigfile])
    #print "Moving", produced.path, "to", dest.path
    gen_pymod= env.LinkInstallAs(dest, produced)
    # this appears to be needed for some reason
    env.Requires(swigr, swiglink)
    module_deps_requires(env, swigr, "swig", [])
    module_deps_requires(env, swigr, "include", [])
    module_requires(env, swigr, 'include')
    patched=penv.IMPModulePatchSWIG(target=['#/build/src/%(module)s_wrap.cpp'%vars],
                               source=['#/build/src/%(module)s_wrap.cpp-in'%vars])
    penv.IMPModulePatchSWIG(target=['#/build/src/%(module)s_wrap.h'%vars],
                       source=['#/build/src/%(module)s_wrap.h-in'%vars])
    lpenv= bug_fixes.clone_env(penv)
    if env['IMP_USE_PCH']:
        if module=='kernel':
            pchh= penv.IMPGeneratePCH(target="#/build/swig/pch.h", source=[])
            bpch= penv.IMPBuildPCH(source=pchh, target="#/build/swig/pch.h.gch")
            env.Alias('pch', [bpch])
            env.Requires(pchh, env.Alias('kernel-include'))
        #lpenv.Prepend(CPPFLAGS=['-include '+env['pch']])
        lpenv.Prepend(CPPPATH=['#/build/swig'])
        lpenv.Prepend(CPPFLAGS=['-include', 'pch.h'])
        lpenv.Prepend(CXXFLAGS=['-Winvalid-pch'])
    buildlib = lpenv.LoadableModule('#/build/lib/_IMP%(module_suffix)s' % get_module_variables(lpenv),
                                    patched)
    #print "Environment", env['CXXFLAGS']
    if env['IMP_USE_PCH']:
        env.Depends(patched, env.Alias('pch'))
    installinit = penv.InstallAs(penv.GetInstallDirectory('pythondir',
                                                          vars['module_include_path'],
                                                          '__init__.py'),
                                 gen_pymod)
    installlib = penv.Install(penv.GetInstallDirectory('pyextdir'), buildlib)
    utility.postprocess_lib(penv, buildlib)
    #build.append(buildlib)
    pybuild.append(gen_pymod)
    pybuild.append(buildlib)
    install.append(installinit)
    install.append(installlib)
    module_alias(env, 'swig', [swigfile]+swiglink)
    for f in pythonfiles:
        #print f
        nm= os.path.split(f.path)[1]
        #print ('#/build/lib/%s/'+nm) % vars['module_include_path']
        pybuild.append(env.LinkInstallAs(('#/build/lib/%s/'+nm) % vars['module_include_path'],
                                         f))
        install.append(env.InstallAs(env.GetInstallDirectory('pythondir',
                                                             vars['module_include_path'],
                                                             nm),f))
    # Install the Python extension and module:
    #buildlib = env.Install("#/build/lib", pyext)
    module_alias(env, 'python', pybuild, True)
    add_to_global_alias(env, 'all', 'python')
    module_alias(env, 'install-python', install)
    add_to_module_alias(env, 'install', 'install-python')
    module_deps_requires(env, install, 'install-python', [])

def IMPModuleGetExamples(env):
    return utility.get_matching_recursive(["*.py","*.readme"])

def IMPModuleGetExampleData(env):
    ret=  utility.get_matching_recursive(["*.pdb", "*.mrc", "*.dat", "*.xml", "*.em", "*.imp", "*.impb",
                                          "*.mol2"])
    return ret

def IMPModuleGetPythonTests(env):
    return utility.get_matching_recursive(["test_*.py"])
def IMPModuleGetCPPTests(env):
    return utility.get_matching_recursive(["test_*.cpp"])


def IMPModuleGetHeaders(env):
    raw_files=utility.get_matching_recursive(["*.h"])
    files=[]
    for f in raw_files:
        s= str(f)
        #print s
        fname= os.path.split(s)[1]
        if fname.startswith("."):
            continue
        if s=="%(module)s_config.h"%get_module_variables(env):
            continue
        files.append(f)
    return files

def IMPModuleGetSwigFiles(env):
    files=utility.get_matching(["IMP_*.i"])
    return files

def IMPModuleGetPython(env):
    files=utility.get_matching(["src/*.py"])
    return files

def IMPModuleGetSources(env):
    raw_files=utility.get_matching_recursive(["*.cpp"])
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
        if s=="%(module)s_config.cpp"%get_module_variables(env):
            continue
        files.append(f)
    return files

def IMPModuleGetData(env):
    raw_files=utility.get_matching_recursive(["*"])
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
    raw_files= utility.get_matching(["*.cpp", "*.py"])
    return raw_files

def IMPModuleGetDocs(env):
    files=utility.get_matching(["*.dox", "*.pdf", "*.dot", "*.png"])
    return files


def IMPModuleDoc(env, files, authors,
                 brief, overview,
                 publications=None,
                 license="standard"):
    build=[]
    install=[]
    docdir=env['docdir']+"/"+get_module_variables(env)['module_include_path']
    build.append(env.IMPModuleMakeModPage(source=[env.Value(authors),
                                             env.Value(brief),
                                             env.Value(overview),
                                             env.Value(publications),
                                             env.Value(license)],
                                     target='generated/overview.dox'))
    for f in files:
        #print "file", str(f)
        if str(f).endswith(".dox") or str(f).endswith(".dot"):
            pass
        else:
            b=env.InstallAs("#/build/doc/html/"+str(f), f)
            #print str(b)
            build.append(b)
            #install.append(env.Install(f, docdir))
    module_alias(env, 'doc', build)
    env.Alias('doc-files', build)
    #add_to_global_alias(env, 'doc', 'doc')
    #module_alias(env, 'install-doc', install)
    #add_to_module_alias(env, 'install', 'install-doc')



#   files= ["#/bin/imppy.sh", "#/tools/run_all_tests.py"]+\
#        [x.abspath for x in Glob("test_*.py")+ Glob("*/test_*.py")]

def IMPModuleTest(env, python_tests, cpp_tests):
    """Pseudo-builder to run tests for an IMP module. The single target is
       generally a simple output file, e.g. 'test.passed', while the single
       source is a Python script to run (usually run-all-tests.py).
       Right now, the assumption is made that run-all-tests.py executes
       all files called test_*.py in the current directory and subdirectories."""
    files= ["#/tools/imppy.sh", "#/scons_tools/run-all-tests.py"]+\
        [x.abspath for x in python_tests]
    files.append(env.Alias(get_module_name(env)+"-python"))
    #print files
    if len(cpp_tests)>0:
        #print "found cpp tests", " ".join([str(x) for x in cpp_tests])
        (build, install_list)= _make_programs(env, [get_module_name(env)]+env.get_module_modules(), False, cpp_tests)
        cpptest= env.IMPModuleCPPTest(target="cpp_test_programs.py",
                                       source= build)
        files.append(cpptest)
    test = env.IMPModuleRunTest(target="test.passed", source=files,
                              TEST_TYPE='unit test')
    env.AlwaysBuild("test.passed")
    module_alias(env, 'test', test)
    add_to_global_alias(env, 'test', 'test')

def check_libraries_and_headers(env, libraries, headers):
    rlibraries=[x for x in libraries]
    rlibraries.reverse();
    def hname(h):
        return h.replace("/","").replace(".", "")
    custom_tests={}
    for l in libraries:
        def libtest(context):
            ret = dependency.check_lib(context, header= headers[0], lib=l)
            if ret[0]:
                env.Append(LIBS=[l])
            context.did_show_result=True
            context.Result(ret[0])
            return ret[0]
        custom_tests["Check"+l]= libtest
    for h in headers:
        def libtest(context):
            ret = context.sconf.CheckHeader(header=h, language='C++')
            context.did_show_result=True
            context.Result(ret)
            return ret
        custom_tests["Check"+hname(h)]= libtest
    conf=env.Configure(custom_tests= custom_tests)
    for l in rlibraries:
        r= eval("conf.Check"+l+"()")
        if not r:
            raise EnvironmentError("the library " +l +" is required by module but could "\
                                       + "not be linked.")
    for h in headers:
        r= eval("conf.Check"+hname(h)+"()")
        if not r:
            raise EnvironmentError("The header "+ h +" is required by module but could "\
                                       + "not be found.")

def IMPModuleBuild(env, version, required_modules=[],
                   lib_only_required_modules=[],
                   optional_modules=[],
                   optional_dependencies=[], config_macros=[],
                   module=None, module_suffix=None,
                   module_include_path=None, module_preproc=None,
                   module_namespace=None, module_nicename=None,
                   required_dependencies=[],
                   cxxflags=[], cppdefines=[], python_docs=False):
    if env.GetOption('help'):
        return

    if module is None:
        module=Dir('.').abspath.split('/')[-1]
    if module_suffix is None:
        module_suffix="_"+module
    if module_include_path is None:
        module_include_path="IMP/"+module
    if module_preproc is None:
        module_preproc="IMP"+module.upper()
    if module_namespace is None:
        module_namespace="IMP::"+module
    if module_nicename is None:
        module_nicename= "IMP."+module
    if python_docs:
        env.Append(IMP_PYTHON_DOCS=[module])
    if module.lower() != module:
        print >> sys.stderr, "Module names must be all lower case. This can change if you complain, but might be complicated to fix. Failed on", module
        env.Exit(1)

    for m in required_modules+lib_only_required_modules:
        if not env.get_module_ok(m):
            print "Module IMP."+module, "disabled due to disabled module "\
                  "IMP."+m
            _set_module_ok(env, module, False)
            return
    for m in required_dependencies:
        if not env.get_dependency_ok(m):
            print "Module IMP."+module, "disabled due to missing dependency "\
                  +m
            _set_module_ok(env, module, False)
            return
    found_optional_modules=env.get_found_modules(optional_modules)
    _set_module_modules(env, module,required_modules+found_optional_modules\
                                     +lib_only_required_modules)
    _set_module_python_modules(env, module,required_modules+env.get_found_modules(optional_modules))
    env['IMP_MODULES_ALL'].append(module)
    _set_module_ok(env, module, True)

    preclone=env
    found_optional_dependencies=env.get_found_dependencies(optional_dependencies)
    unfound=[]
    for d in optional_dependencies:
        if not d in found_optional_dependencies:
            unfound.append(d)
    _set_module_unfound_dependencies(env, module, unfound)
    _set_module_dependencies(env, module, found_optional_dependencies\
                             +required_dependencies)

    _set_module_version(env, module, version,
                        found_optional_modules+found_optional_dependencies);

    env = bug_fixes.clone_env(env)
    _set_module_name(env, module)

    vars={'module_include_path':module_include_path,
          'module':module,
          'PREPROC':module_preproc,
          'namespace':module_namespace,
          'module_suffix':module_suffix,
          'module_nicename':module_nicename}
    env['IMP_MODULE_VARS']=vars


    build_config=[]
    if cxxflags:
        env.Replace(CXXFLAGS=cxxflags)
    if cppdefines:
        env.Append(CPPDEFINES=cppdefines)

    module_alias(env, 'config', build_config)

    print "Configuring module IMP." + get_module_name(env)+" version "+env.get_module_version()

    if len(required_modules+required_dependencies)>0:
        print "  (requires " +", ".join(required_modules+required_dependencies) +")"
    #if len(found_optional_modules + found_optional_dependencies)>0:
    #    print "  (using " +", ".join(found_optional_modules + found_optional_dependencies) +")"

    env['IMP_MODULE_CONFIG']=config_macros

    env.SConscript('doc/SConscript', exports='env')
    env.SConscript('examples/SConscript', exports='env')
    env.SConscript('data/SConscript', exports='env')

    env.SConscript('include/SConscript', exports='env')
    env.SConscript('src/SConscript', exports='env')
    env.SConscript('bin/SConscript', exports='env')
    if env['IMP_PROVIDE_PYTHON']:
        env.SConscript('pyext/SConscript', exports='env')
        env.SConscript('test/SConscript', exports='env')

    add_to_global_alias(env, 'install', 'install')
    return env
