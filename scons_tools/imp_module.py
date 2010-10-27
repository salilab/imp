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
import checks
import modpage
import pch

from SCons.Script import Builder, File, Action, Glob, Return, Alias, Dir, Move, Copy

#def module_depends(env, target, source):
#    env.Depends(target, [env.Alias(env['IMP_MODULE']+"-"+source)])

def file_compare(a, b):
    """Check if two files are the same, by comparing the path"""
    pa= a.abspath
    pb= b.abspath
    return cmp(pa,pb)

def module_glob(patterns):
    """provide a canonical list of files which match the passed list of patterns.
    Otherwise changes in the ordering will cause scons to rebuild things."""
    ret=[]
    for x in patterns:
        ret= ret+Glob(x, ondisk=True)
    ret.sort()#cmp= file_compare)
    return ret

def module_requires(env, target, source):
    """Make sure that 'module-source' is built before 'module-target'"""
    for t in target:
        env.Requires(t, [env.Alias(env['IMP_MODULE']+"-"+source)])


def module_depends(env, target, source):
    """Make sure that 'module-source' is built before 'module-target'"""
    env.Depends(target, [env.Alias(env['IMP_MODULE']+"-"+source)])

def module_alias(env, target, source, is_default=False):
    """Add an alias called 'module-target' which builds source"""
    a=env.Alias(env['IMP_MODULE']+"-"+target, [source])
    if is_default:
        env.Default(a)

def add_to_global_alias(env, target, source):
    """Add the module alias 'module-source' to the global alias list 'target'"""
    env.Alias(env.Alias(target), [env.Alias(env['IMP_MODULE']+'-'+source)])


def add_to_module_alias(env, target, source):
    """Add the alias 'module-source' to the alias 'module-target'"""
    env.Alias(env.Alias(env['IMP_MODULE']+'-'+target),
              [env.Alias(env['IMP_MODULE']+'-'+source)])


def module_deps_requires(env, target, source, dependencies):
    """For each of the module dependency make sure that 'moduledep-source'
    is built before 'target'"""
    #print "alldeps is " +str(dependencies + env['IMP_REQUIRED_MODULES'])
    for d in dependencies + env['IMP_REQUIRED_LIB_MODULES']:
        #print str(target) + " requires " + d+'-'+source
        env.Requires(target, env.Alias(d+'-'+source))
    if env['IMP_MODULE'] != 'kernel':
        env.Requires(target, env.Alias("kernel-"+source))



def module_deps_depends(env, target, source, dependencies):
    """For each of the module dependency make sure that 'moduledep-source'
    is built before 'target'"""
    #print "alldeps is " +str(dependencies + env['IMP_REQUIRED_MODULES'])
    for d in dependencies + env['IMP_REQUIRED_LIB_MODULES']:
        #print str(target) + " requires " + d+'-'+source
        env.Depends(target, env.Alias(d+'-'+source))


def expand_dependencies(env, deps, is_kernel=False):
    """Recursively expand the list of dependencies. The dependencies are returned in order of the all_modules environment variable."""
    size=-1
    all=[]
    #print "expanding ", deps
    to_expand=deps
    expanded={}
    for i in to_expand:
        expanded[i]=True
    while len(to_expand) != 0:
        c= to_expand[-1]
        try:
            ndeps=env[c+"_required_modules"]
        except:
            print >> sys.stderr, "Modules can only depend on modules which are configured before them."
            print >> sys.stderr, "Specifically, module "+str(env['IMP_MODULE']) \
                  + " cannot depend on module " +c
            raise ValueError("Bad module depedency")
        ndeps.reverse()
        to_expand=to_expand[:-1]
        for i in ndeps:
            if not expanded.has_key(i):
                expanded[i]=True
                to_expand.append(i)
        #print c, to_expand, expanded
        all.append(c)
    filtered=[]
    for i in env['all_modules']:
        try:
            all.index(i)
        except:
            pass
        else:
            filtered.append(i)
    # always depend on kernel
    filtered.reverse()
    if not is_kernel:
        filtered.append("kernel")
    #all.sort()
    #print "got", filtered
    #print "expanded "+str(deps) + " to get "+str(filtered)
    return filtered


def dependencies_to_libs(env, deps, is_kernel=False):
    libs=[]
    deps = deps
    ed=expand_dependencies(env,deps, is_kernel)
    if not env['fastlink']:
        for d in ed:
            if d== 'kernel':
                libs.append("imp")
            else:
                libs.append("imp_"+d)
    for d in ed:
        #print "libs for " + d + " are " + str(env[d+"_libs"])
        try:
            libs= libs+env[d+"_libs"]
        except:
            pass
    return libs

def clean_libs(libs):
    ret=[]
    for i,l in enumerate(libs):
        if l not in libs[i+1:]:
            ret.append(l)
    return ret

def do_mac_name_thing(env, source, target):
    """Set the names and paths for the mac libraries based on the current locations
    of the libs."""
    targetdir= os.path.split(target[0].abspath)[0]
    sourcedir= os.path.split(source[0].abspath)[0]
    #print targetdir
    #print sourcedir
    env.Execute("install_name_tool -id %s %s"% (target[0].abspath, target[0].abspath))
    env.Execute("install_name_tool -change %s %s %s"%(os.path.join(sourcedir, 'libimp.dylib'),
                                                      os.path.join(targetdir, 'libimp.dylib'),
                                                      target[0].abspath))
    for m in env['IMP_MODULES_ALL']:
        oname=os.path.join(sourcedir, "libimp_"+m+".dylib")
        nname=os.path.join(targetdir, "libimp_"+m+".dylib")
        #print oname
        #print nname
        env.Execute("install_name_tool -change %s %s %s"%(oname,
                                                          nname,
                                                          target[0].abspath))
def postprocess_lib(env, target):
    """ for now assume that all libs go in the same place"""
    if env['PLATFORM'] == 'darwin':
        dir= os.path.split(target[0].abspath)[0]
        env.AddPostAction(target, do_mac_name_thing)


def make_static_build(env):
    """Make the build static if appropriate"""
    if env['CC'] == 'gcc':
        env.Append(LINKFLAGS=['-static'])
    else:
        print >> sys.stderr, "WARNING: Static builds only supported with GCC, ignored."

def unmake_static_build(env):
    """Make the build static if appropriate"""
    if env['CC'] == 'gcc':
        lf= env['LINKFLAGS']
        lf.remove('-static')
        env.Replace(LINKFLAGS=lf)
    else:
        print >> sys.stderr, "WARNING: Static builds only supported with GCC, ignored."


def make_vars(env):
    """Make a map which can be used for all string substitutions"""
    module = env['IMP_MODULE']
    module_include_path = env['IMP_MODULE_INCLUDE_PATH']
    module_src_path = env['IMP_MODULE_SRC_PATH']
    module_preproc = env['IMP_MODULE_PREPROC']
    module_namespace = env['IMP_MODULE_NAMESPACE']
    module_suffix = env['IMP_MODULE_SUFFIX']
    version = env['IMP_MODULE_VERSION']#source[1].get_contents()
    nicename= env['IMP_MODULE_NICENAME']
    author = nicename+" development team"
    vars={'module_include_path':module_include_path,
          'module_src_path':module_src_path, 'module':module,
          'PREPROC':module_preproc, 'author':author, 'version':version,
          'namespace':module_namespace,
          'module_suffix':module_suffix,
          'module_nicename':nicename}
    return vars


def IMPModuleLib(envi, files):
    """Build, and optionally also install, an IMP module's C++
       shared library. This is only available from within an environment
       created by `IMPSharedLibraryEnvironment`."""
    from scons_tools import get_sharedlib_environment, get_staticlib_environment
    vars= make_vars(envi)
    module = envi['IMP_MODULE']
    module_suffix = envi['IMP_MODULE_SUFFIX']
    vars= make_vars(envi)
    if envi['build']=="debug" and envi['linktest']:
        link= envi.IMPModuleLinkTest(target=['#/build/src/%(module)s_link_0.cpp'%vars, '#/build/src/%(module)s_link_1.cpp'%vars], source=[])
        files= files+link
    version= envi['IMP_MODULE_VERSION']
    config= envi.IMPModuleConfigCPP(target=["#/build/src/%(module)s_config.cpp"%vars],
                                   source=[envi.Value(version),
                                           envi.Value(envi.subst(envi['datadir'])),
                                           envi.Value(envi.subst(os.path.join(envi['docdir'], "examples")))])
    #env.AlwaysBuild(version)
    files =files+ config
    deps=clean_libs(dependencies_to_libs(envi, envi[envi['IMP_MODULE']+"_required_modules"],
                                          envi['IMP_MODULE'] == 'kernel')\
                    +envi[envi['IMP_MODULE']+"_libs"])
    build=[]
    if envi['IMP_BUILD_STATIC']:
        env= get_staticlib_environment(envi)
        env.Prepend(LIBS=deps)
        build.append( env.StaticLibrary('#/build/lib/imp%s' % module_suffix,
                                      list(files)))
    if envi['IMP_BUILD_DYNAMIC']:
        env = get_sharedlib_environment(envi, '%(PREPROC)s_EXPORTS' % vars,
                                    cplusplus=True)
        env.Prepend(LIBS=deps)
        build.append(env.SharedLibrary('#/build/lib/imp%s' % module_suffix,
                                       list(files) ) )
        postprocess_lib(env, build[-1])
    install=[]
    for b in build:
        install.append(envi.Install(env.GetInstallDirectory('libdir'), b) )
    postprocess_lib(envi, install[-1])
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
    vars=make_vars(env)
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
    vars=make_vars(env)
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
    vars=make_vars(env)
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

def _make_programs(envi, required_modules, extra_libs, install, files):
    from scons_tools import get_bin_environment
    env= get_bin_environment(envi)
    vars=make_vars(env)
    if env['fastlink']:
        if env['IMP_MODULE'] != "kernel":
            env.Prepend(LINKFLAGS=['-limp_'+env['IMP_MODULE']])
        else:
            env.Prepend(LINKFLAGS=['-limp'])
    env.Prepend(LIBS=clean_libs(dependencies_to_libs(env, env[env['IMP_MODULE']+"_required_modules"]\
                                                    +required_modules,
                                                env['IMP_MODULE'] == 'kernel')\
                          +env[env['IMP_MODULE']+"_libs"]))
    if not env['fastlink']:
        env.Prepend(LIBS=['imp%(module_suffix)s' % vars])
    env.Append(LIBS=extra_libs);
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

def IMPModuleBin(env, files, required_modules=[], extra_libs=[], install=True):
    (build, install_list)= _make_programs(env, required_modules, extra_libs, install, files)
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


def IMPModulePython(env, swigfiles=[], pythonfiles=[]):
    """Build and install an IMP module's Python extension and the associated
       wrapper file from a SWIG interface file. This is only available from
       within an environment created by `IMPPythonExtensionEnvironment`."""
    from scons_tools import get_pyext_environment
    module = env['IMP_MODULE']
    module_suffix= env['IMP_MODULE_SUFFIX']
    vars=make_vars(env)
    pybuild=[]
    install=[]
    penv = get_pyext_environment(env, module.upper(), cplusplus=True)
    penv.Decider('timestamp-match')
    if penv['CC'] != 'w32cc':
        penv['LIBS']=[]
    else:
        # windows needs all of the IMP modules linked in explicitly
        penv.Prepend(LIBS=clean_libs(dependencies_to_libs(env, env[env['IMP_MODULE']+"_required_modules"],
                                               env['IMP_MODULE'] == 'kernel')\
                         +env[env['IMP_MODULE']+"_libs"]))
    penv.Prepend(LIBS=['imp%s' % module_suffix])
    swigfile= penv._IMPSWIGPreface(target=[File("#/build/swig/IMP_%(module)s.i"%vars)],
                                   source=[File("swig.i-in"),
                                           env.Value(env['IMP_REQUIRED_PYTHON_MODULES']),
                                           env.Value(env['IMP_MODULE_VERSION']),
                                           #python supports serialization of object, why on earth do they just convert them to strings?
                                           env.Value(" ".join(env[module+"_optional_dependencies"])),])
    swiglink=[]
    for i in swigfiles:
        if str(i).find('/')==-1:
            swiglink.append( env.LinkInstallAs("#/build/swig/"+str(i), i) )
    dest = File('#/build/lib/%(module_include_path)s/__init__.py' % vars)
    produced=File("#/build/src/"+vars['module_include_path'].replace("/",".")+".py")
    swig=penv._IMPSWIG(target=[produced, '#/build/src/%(module)s_wrap.cpp-in'%vars,
                               '#/build/src/%(module)s_wrap.h-in'%vars],
                       source=swigfile)
    #print "Moving", produced.path, "to", dest.path
    gen_pymod= env.LinkInstallAs(dest, produced)
    # this appears to be needed for some reason
    env.Requires(swig, swiglink)
    module_deps_requires(env, swig, "swig", [])
    module_deps_requires(env, swig, "include", [])
    module_requires(env, swig, 'include')
    patched=penv._IMPPatchSWIG(target=['#/build/src/%(module)s_wrap.cpp'%vars],
                               source=['#/build/src/%(module)s_wrap.cpp-in'%vars])
    penv._IMPPatchSWIG(target=['#/build/src/%(module)s_wrap.h'%vars],
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
    buildlib = lpenv.LoadableModule('#/build/lib/_IMP%s' % module_suffix,
                                    patched)
    #print "Environment", env['CXXFLAGS']
    if env['IMP_USE_PCH']:
        env.Depends(patched, env.Alias('pch'))
    installinit = penv.InstallAs(penv.GetInstallDirectory('pythondir',
                                                          vars['module_include_path'],
                                                          '__init__.py'),
                                 gen_pymod)
    installlib = penv.Install(penv.GetInstallDirectory('pyextdir'), buildlib)
    postprocess_lib(penv, buildlib)
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
    return module_glob(["*.py", "*/*.py","*.readme","*/*.readme"])

def IMPModuleGetExampleData(env):
    ret=  module_glob(["*.pdb", "*.mrc", "*.dat", "*.xml", "*.em", "*.imp", "*.impb",
                         "*/*.pdb", "*/*.mrc", "*/*.dat", "*/*.xml", "*/*.em", "*/*.imp", "*/*.impb",
                       "*/*/*.pdb", "*/*/*.mrc", "*/*/*.dat", "*/*/*.xml", "*/*/*.em","*/*/*.imp", "*/*/*.impb",])
    return ret

def IMPModuleGetPythonTests(env):
    return module_glob(["test_*.py", "*/test_*.py"])
def IMPModuleGetCPPTests(env):
    return module_glob(["test_*.cpp", "*/test_*.cpp"])


def IMPModuleGetHeaders(env):
    vars = make_vars(env)
    raw_files=module_glob(["*.h", "*/*.h"])
    files=[]
    for f in raw_files:
        s= str(f)
        #print s
        fname= os.path.split(s)[1]
        if fname.startswith("."):
            continue
        if s=="%(module)s_config.h"%vars:
            continue
        files.append(f)
    return files

def IMPModuleGetSwigFiles(env):
    vars = make_vars(env)
    files=module_glob(["IMP_*.i"])
    return files

def IMPModuleGetPython(env):
    vars = make_vars(env)
    files=module_glob(["src/*.py"])
    return files

def IMPModuleGetSources(env):
    vars = make_vars(env)
    raw_files=module_glob(["*.cpp", "*/*.cpp"])
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
        if s=="%(module)s_config.cpp"%vars:
            continue
        files.append(f)
    return files

def IMPModuleGetData(env):
    vars = make_vars(env)
    raw_files=module_glob(["*"])
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
    vars = make_vars(env)
    raw_files= module_glob(["*.cpp", "*.py"])
    return raw_files

def IMPModuleGetDocs(env):
    files=module_glob(["*.dox", "*.pdf", "*.dot", "*.png"])
    return files


def IMPModuleDoc(env, files, authors,
                 brief, overview,
                 publications=None,
                 license="standard"):
    vars= make_vars(env)
    build=[]
    install=[]
    docdir=env['docdir']+"/"+vars['module_include_path']
    build.append(env._IMPMakeModPage(source=[env.Value(authors),
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
            b=env.InstallAs("#/doc/html/"+str(f), f)
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

def IMPModuleTest(env, python_tests, cpp_tests, cpp_required_modules=[],
                  cpp_extra_libs=[]):
    """Pseudo-builder to run tests for an IMP module. The single target is
       generally a simple output file, e.g. 'test.passed', while the single
       source is a Python script to run (usually run-all-tests.py).
       Right now, the assumption is made that run-all-tests.py executes
       all files called test_*.py in the current directory and subdirectories."""
    files= ["#/tools/imppy.sh", "#/scons_tools/run-all-tests.py"]+\
        [x.abspath for x in python_tests]
    files.append(env.Alias(env['IMP_MODULE']+"-python"))
    #print files
    if len(cpp_tests)>0:
        #print "found cpp tests", " ".join([str(x) for x in cpp_tests])
        (build, install_list)= _make_programs(env, cpp_required_modules, cpp_extra_libs, False, cpp_tests)
        cpptest= env._IMPModuleCPPTest(target="cpp_test_programs.py",
                                       source= build)
        files.append(cpptest)
    test = env._IMPModuleTest(target="test.passed", source=files,
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
            ret = checks.check_lib(context, header= headers[0], lib=l)
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

def process_dependencies(env, dependencies, required=False):
    m_libs=[]
    missing=[]
    for d in dependencies:
        nd= checks.nicename(d)
        if d== "modeller":
            if not env.get('HAS_MODELLER', False):
                missing.append(d)
        else:
            if env.get(nd.upper()+"_LIBS", "not found")=="not found":
                raise ValueError("Do not understand dependency: " +d)
            elif env[nd.upper()+"_LIBS"]:
                m_libs=m_libs+env[nd.upper()+"_LIBS"]
            else:
                missing.append(d)
    if required and len(missing)>0:
        #print "  (missing "+ ", ".join(missing)+", disabled)"
        raise EnvironmentError("missing dependency "+", ".join(missing))
    return (m_libs, missing)

def IMPModuleBuild(env, version, required_modules=[],
                   lib_only_required_modules=[],
                   optional_modules=[],
                   optional_dependencies=[], config_macros=[],
                   module=None, module_suffix=None,
                   module_include_path=None, module_src_path=None, module_preproc=None,
                   module_namespace=None, module_nicename=None,
                   required_dependencies=[],
                   variabsle=None,
                   required_libraries=[], required_headers=[],
                   cxxflags=[], cppdefines=[], python_docs=False):
    if module is None:
        module=Dir('.').abspath.split('/')[-1]
    if module_suffix is None:
        module_suffix="_"+module
    if module_src_path is None:
        module_src_path="modules/"+module
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
    env['IMP_MODULE'] = module
    env['IMP_MODULE_SUFFIX'] = module_suffix
    env['IMP_MODULE_INCLUDE_PATH'] = module_include_path
    env['IMP_MODULE_SRC_PATH'] = module_src_path
    env['IMP_MODULE_PREPROC'] = module_preproc
    env['IMP_MODULE_NAMESPACE'] = module_namespace
    env['IMP_MODULE_NICENAME'] = module_nicename
    #env['IMP_MODULE_VERSION'] = "SVN"
    env['IMP_MODULE_AUTHOR'] = "A. Biologist"

    # Check required modules and add kernel
    for x in required_modules+optional_modules+lib_only_required_modules:
        if x.startswith("imp_"):
            print >> sys.stderr, "Required modules should have the name of the module (eg 'algebra'), not the name of the library."
            print >> sys.stderr, x
            env.Exit(1)
        if x=='kernel':
            print >> sys.stderr, "You do not need to list the kernel as a required module"
            print >> sys.stderr, x
            env.Exit(1)
        #required_modules.append('kernel')
    if module.lower() != module:
        print >> sys.stderr, "Module names must be all lower case. This can change if you complain, but might be complicated to fix. Failed on", module
        env.Exit(1)
    #print module_suffix
    #print module_src_path
    #print module_include_path
    #print module_preproc
    #print module_namespace
    found_optional_modules=[]
    for x in optional_modules:
        if env.get(x+"_ok", False):
            found_optional_modules.append(x)
    env[module+"_required_modules"]=required_modules+found_optional_modules\
                                     +lib_only_required_modules
    env[module+"_optional_dependencies"]= optional_dependencies
    env['IMP_MODULES_ALL'].append(module)

    module_failure = None
    preclone=env
    #if not env.GetOption('clean') and not env.GetOption('help'):
    if len(required_libraries)+len(required_headers) > 0:
        try:
            check_libraries_and_headers(env, required_libraries, required_headers)
        except EnvironmentError, e:
            module_failure = e

    if env.GetOption('help'):
        return

    env.Prepend(SCANNERS = [swig.scanner, swig.inscanner])
    env['all_modules'].append(module)
    processed_optional_dependencies=process_dependencies(env, optional_dependencies)
    try:
        m_libs=processed_optional_dependencies[0]\
            + process_dependencies(env, required_dependencies, True)[0]
    except EnvironmentError, e:
        env[module+"_libs"]=[]
        env = bug_fixes.clone_env(env)
        module_failure = e
    else:
        env[module+"_libs"]=m_libs
        env = bug_fixes.clone_env(env)
    for m in required_modules+lib_only_required_modules:
        if not env.get(m+"_ok", False):
            module_failure = "module "+m+" not supported"
    env['IMP_REQUIRED_PYTHON_MODULES']= required_modules+found_optional_modules
    env['IMP_REQUIRED_LIB_MODULES']= lib_only_required_modules\
                                     +required_modules+found_optional_modules
    if env['fastlink']:
        ed= expand_dependencies(env,env['IMP_REQUIRED_LIB_MODULES'], module=='kernel')
        for m in ed:
            if m != 'kernel':
                env.Append(LINKFLAGS=['-limp_'+m])
            else:
                env.Append(LINKFLAGS=['-limp'])
    env.Append(BUILDERS = {'IMPModuleConfigH': config_h.ConfigH,
                           'IMPModuleConfigCPP': config_h.ConfigCPP,
                           'IMPModuleLinkTest': link_test.LinkTest,
                           'IMPGeneratePCH': pch.GeneratePCH,
                           'IMPBuildPCH': pch.BuildPCH})
    env.Prepend(CPPPATH=['#/build/include'])
    env.Prepend(LIBPATH=['#/build/lib'])
    if cxxflags:
        env.Append(CXXFLAGS=cxxflags)
    if cppdefines:
        env.Append(CPPDEFINES=cppdefines)
    build_config=[]
    # Generate version information
    env.AddMethod(IMPModuleLib)
    env.AddMethod(IMPModuleInclude)
    module_alias(env, 'config', build_config)
    env.AddMethod(IMPModuleData)
    env.AddMethod(IMPModulePython)
    env.AddMethod(IMPModuleTest)
    env.AddMethod(IMPModuleBuild)
    env.AddMethod(IMPModuleGetHeaders)
    env.AddMethod(IMPModuleGetExamples)
    env.AddMethod(IMPModuleGetExampleData)
    env.AddMethod(IMPModuleGetPythonTests)
    env.AddMethod(IMPModuleGetCPPTests)
    env.AddMethod(IMPModuleGetData)
    env.AddMethod(IMPModuleGetSources)
    env.AddMethod(IMPModuleGetPython)
    env.AddMethod(IMPModuleGetSwigFiles)
    env.AddMethod(IMPModuleGetBins)
    env.AddMethod(IMPModuleBin)
    env.AddMethod(IMPModuleDoc)
    env.AddMethod(IMPModuleExamples)
    env.AddMethod(IMPModuleGetDocs)
    env.AddMethod(modpage.Publication)
    env.AddMethod(modpage.Website)
    env.AddMethod(modpage.StandardPublications)
    env.AddMethod(modpage.StandardLicense)
    env.Append(BUILDERS={'_IMPModuleTest': test.UnitTest})
    env.Append(BUILDERS={'_IMPModuleCPPTest': test.CPPTestHarness})
    env.Append(BUILDERS={'_IMPColorizePython': examples.ColorizePython})
    env.Append(BUILDERS={'_IMPExamplesDox': examples.MakeDox})
    env.Append(BUILDERS={'_IMPSWIG': swig.SwigIt})
    env.Append(BUILDERS={'_IMPPatchSWIG': swig.PatchSwig})
    env.Append(BUILDERS={'_IMPSWIGPreface': swig.SwigPreface})
    env.Append(BUILDERS={'_IMPMakeModPage': modpage.MakeModPage})
    env.Append(BUILDERS={'IMPRun': run.Run})

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

    deps= ", ".join([x for x in optional_dependencies if env[checks.nicename(x).upper()+"_LIBS"]])
    if len(deps)>0:
        version=version+" with "+deps
    env['IMP_MODULE_VERSION'] = version


    if module_failure is not None:
        print "IMP."+env['IMP_MODULE']+" is disabled due to", str(module_failure)
        #preclone.Append(IMP_BUILD_SUMMARY=["IMP."+module+" disabled"])
        test.disabled_modules.append(module)
        preclone[module+"_ok"]=False
        Return()
    else:
        print "Configuring module IMP." + env['IMP_MODULE']+" version "+env['IMP_MODULE_VERSION']
        preclone[module+"_ok"]=True



    nice_deps = expand_dependencies(env,env['IMP_REQUIRED_LIB_MODULES'], env['IMP_MODULE'] == 'kernel')
    #print "nice is "+str(nice_deps)
    all_deps=["IMP."+x for x in nice_deps if x is not "kernel"]+required_libraries
    if len(all_deps) > 0:
        nice_deps.remove('kernel')
        print "  (requires " +", ".join(all_deps) +")"

    env['IMP_MODULE_CONFIG']=config_macros

    vars=make_vars(env)
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
