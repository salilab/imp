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
import modeller_test
import run
import checks
import modpage
import pch

from SCons.Script import Builder, File, Action, Glob, Return, Alias, Dir

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
        ret= ret+Glob(x)
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
    for d in dependencies + env['IMP_REQUIRED_MODULES']:
        #print str(target) + " requires " + d+'-'+source
        env.Requires(target, env.Alias(d+'-'+source))
    if env['IMP_MODULE'] != 'kernel':
        env.Requires(target, env.Alias("kernel-"+source))



def module_deps_depends(env, target, source, dependencies):
    """For each of the module dependency make sure that 'moduledep-source'
    is built before 'target'"""
    #print "alldeps is " +str(dependencies + env['IMP_REQUIRED_MODULES'])
    for d in dependencies + env['IMP_REQUIRED_MODULES']:
        #print str(target) + " requires " + d+'-'+source
        env.Depends(target, env.Alias(d+'-'+source))


def expand_dependencies(env, deps):
    """Recursively expand the list of dependencies and make sure each dependency is only in the list once.
    In an effort to make static linking work, only the last copy in the list of dependencies is kept"""
    size=-1
    all=[]
    for d in deps:
        try:
            ndeps=env[d+"_required_modules"]
            all.append(d)
            for nd in ndeps:
                #print "trying " +str(nd)
                nndeps= expand_dependencies(env, [nd])
                all.append(nd)
                app= all+nndeps
        except:
            print >> sys.stderr, "Module binaries can only depend on modules which are configured before them."
            print >> sys.stderr, "Specifically module "+str(env['IMP_MODULE']) + " cannot depenend on module " +d
            raise ValueError("Bad bin depedency")
    filtered=[]
    #print "all is " + str(all)
    for i in range(0, len(all)):
        v= all[i]
        try:
            all[i+1:].index(v)
        except:
            filtered.append(v)
    # always depend on kernel
    if env['IMP_MODULE'] != 'kernel':
        filtered.append("kernel")
    #all.sort()
    #print "expanded "+str(deps) + " to get "+str(filtered)
    return filtered


def dependencies_to_libs(env, deps):
    libs=[]
    deps = env[env['IMP_MODULE']+"_required_modules"] + deps
    ed=expand_dependencies(env,deps)
    for d in ed:
        if d== 'kernel':
            libs.append("imp")
        else:
            libs.append("imp_"+d)
    for d in ed:
        #print "libs for " + d + " are " + str(env[d+"_libs"])
        libs= libs+env[d+"_libs"]
    return libs+env[env['IMP_MODULE']+"_libs"]



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
        print "Static builds only supported with GCC, ignored."

def unmake_static_build(env):
    """Make the build static if appropriate"""
    if env['CC'] == 'gcc':
        lf= env['LINKFLAGS']
        lf.remove('-static')
        env.Replace(LINKFLAGS=lf)
    else:
        print "Static builds only supported with GCC, ignored."


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
    from scons_tools import get_sharedlib_environment
    vars= make_vars(envi)
    env = get_sharedlib_environment(envi, '%(PREPROC)s_EXPORTS' % vars,
                                    cplusplus=True)
    module = env['IMP_MODULE']
    module_suffix = env['IMP_MODULE_SUFFIX']
    vars= make_vars(env)
    link= env.IMPModuleLinkTest(target=['internal/link_0.cpp', 'internal/link_1.cpp'], source=[])
    config= env.IMPModuleConfigCPP(target=['config.cpp'],
                                   source=[env.Value(env['IMP_MODULE_VERSION'])])
    #env.AlwaysBuild(version)
    files =files+link+ config
    env.Prepend(LIBS=dependencies_to_libs(env, []))
    build=[]
    if env['static'] and env['CC'] == 'gcc':
        build.append( env.StaticLibrary('#/build/lib/imp%s' % module_suffix,
                                      list(files)))
    build.append(env.SharedLibrary('#/build/lib/imp%s' % module_suffix,
                                  list(files) ) )
    postprocess_lib(env, build[-1])
    install=[]
    for b in build:
        install.append(env.Install(env.GetInstallDirectory('libdir'), b) )
    postprocess_lib(env, install[-1])
    module_requires(env, build, 'include')
    module_requires(env, build, 'data')
    module_alias(env, 'lib', build, True)
    add_to_global_alias(env, 'all', 'lib')
    module_alias(env, 'install-lib', install)
    add_to_module_alias(env, 'install', 'install-lib')
    module_deps_requires(env, build, 'include', [])
    module_deps_requires(env, build, 'lib', [])
    module_deps_requires(env, install, 'install-lib', [])
    env.Requires(build, '#/tools/imppy.sh')


def IMPModuleInclude(env, files):
    """Install the given header files, plus any auto-generated files for this
       IMP module."""
    vars=make_vars(env)
    includedir = env.GetInstallDirectory('includedir')

    # Generate config header and SWIG equivalent
    config=env.IMPModuleConfigH(target=['config.h'],
    source=[env.Value(env['IMP_MODULE_CONFIG'])])
    files=files+config
    install = hierarchy.InstallHierarchy(env, includedir+"/"+vars['module_include_path'],
                                         list(files))
    build=hierarchy.InstallHierarchy(env, "#/build/include/"+vars['module_include_path'],
                                     list(files), True)
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


def IMPModuleExamples(env, files):
    vars=make_vars(env)
    #for f in files:
    #    print f.abspath
    if vars['module']== 'kernel':
        path=""
    else:
        path=vars['module']
    (dox, build, install, test)= examples.handle_example_dir(env, Dir("."), vars['module'], path, files)
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

def IMPModuleBin(envi, files, required_modules=[], extra_libs=[], install=True):
    from scons_tools import get_bin_environment
    env= get_bin_environment(envi)
    vars=make_vars(env)
    env.Prepend(LIBS=(['imp%(module_suffix)s' % vars]+dependencies_to_libs(env, required_modules)))
    env.Append(LIBS=extra_libs);
    build=[]
    install_list=[]
    bindir = env.GetInstallDirectory('bindir')
    allprogs=[]
    for f in files:
        prog= env.Program(f)
        allprogs.append(prog)
        cb= env.Install("#/build/bin", prog)
        build.append(cb)
        if install:
            ci= env.Install(bindir, prog)
            install_list.append(ci)
        build.append(prog)
    envi['IMP_MODULE_BINS']= allprogs
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
    if env['IMP_MODULE_CPP']:
        penv = get_pyext_environment(env, module.upper(), cplusplus=True)
        if penv['CC'] != 'w32cc':
            penv['LIBS']=[]
        else:
            # windows needs all of the IMP modules linked in explicitly
            penv.Prepend(LIBS=dependencies_to_libs(env, []))
        penv.Prepend(LIBS=['imp%s' % module_suffix])
        #penv.Append(CPPPATH=[Dir('#').abspath])
        #penv.Append(SWIGFLAGS='-python -c++ -naturalvar')
        swigfile= penv._IMPSWIGPreface(target=[File("#/build/swig/IMP_%(module)s.i"%vars)],
                                      source=[File("swig.i-in"),
                                              env.Value(env['IMP_REQUIRED_MODULES']),
                                              env.Value(env['IMP_MODULE_VERSION'])])
        swiglink=[]
        #print [str(x) for x in interfaces]
        for i in swigfiles:
            swiglink.append( env.LinkInstallAs("#/build/swig/"+str(i), i) )
        gen_pymod = File('IMP%s.py' % module_suffix.replace("_","."))
        swig=penv._IMPSWIG(target=[gen_pymod, 'wrap.cpp-in',
                              'wrap.h-in'],
                           source=swigfile)
        # this appears to be needed for some reason
        env.Requires(swig, swiglink)
        module_deps_requires(env, swig, "swig", [])
        module_deps_requires(env, swig, "include", [])
        module_requires(env, swig, 'include')
        penv._IMPPatchSWIG(target=['wrap.cpp'],
                           source=['wrap.cpp-in'])
        penv._IMPPatchSWIG(target=['wrap.h'],
                           source=['wrap.h-in'])
        lpenv= penv.Clone()
        if env['use_pch']:
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
                                       "wrap.cpp")
        if env['use_pch']:
            env.Depends(buildlib, env.Alias('pch'))
        # Place the generated Python wrapper in lib directory:
        buildinit = penv.LinkInstallAs('#/build/lib/%s/__init__.py'
                                       % vars['module_include_path'],
                                       gen_pymod)
        # Make sure we have example/data files installed, in case someone says
        # import IMP.foo; IMP.foo.get_example_file('foo'); IMP.get_data_path('')
        if module != 'kernel':
            penv.Requires(buildinit, '#/build/doc/examples/%s' \
                          % module)
            penv.Requires(buildinit, '#/build/data/%s' % module)
        else:
            penv.Requires(buildinit, '#/build/data')
        installinit = penv.InstallAs(penv.GetInstallDirectory('pythondir',
                                                            vars['module_include_path'],
                                                            '__init__.py'),
                                    gen_pymod)
        installlib = penv.Install(penv.GetInstallDirectory('pyextdir'), buildlib)
        postprocess_lib(penv, buildlib)
        #build.append(buildlib)
        pybuild.append(buildinit)
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

def IMPModuleGetExamples(env, priority_files=None):
    vars= make_vars(env)
    if not priority_files:
        priority_files = list()
    try:
        files = [Glob(f)[0] for f in priority_files]
    except:
        raise Exception, "An exception has occured in IMPModuleGetExamples. Please do make sure that priority_files are present in their intended location."
    file_set = set([f.abspath for f in files])
    for file in module_glob(["*.py", "*/*.py","*.readme","*/*.readme",
      "*.pdb", "*.mrc"]):
        if not file.abspath in file_set:
            files.append(file)
            file_set.add(file.abspath)
    return files

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
        if s=="config.h":
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
        if s=="config.cpp":
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
    raw_files= module_glob(["*.cpp"])
    return raw_files

def IMPModuleGetDocs(env):
    files=module_glob(["*.dox", "*.pdf", "*.dot"])
    return files


def IMPModuleDoc(env, files, authors,
                 brief, overview,
                 publications=None,
                 license="standard"):
    vars= make_vars(env)
    build=[]
    #install=[]
    docdir=env['docdir']+"/"+vars['module_include_path']
    build.append(env._IMPMakeModPage(source=[env.Value(authors),
                                             env.Value(brief),
                                             env.Value(overview),
                                             env.Value(publications),
                                             env.Value(license)],
                                     target='.generated/overview.dox'))
    for f in files:
        if f== "overview.dox.in":
            raise ValueError("overview.dox-in is added automatically to files list")
        if str(f).endswith(".dox") or str(f).endswith(".dot"):
            pass
        else:
            build.append(env.Install(f, "#/doc/html/"+vars['module']))
            #install.append(env.Install(f, docdir))
    module_alias(env, 'doc', build)
    add_to_global_alias(env, 'all', 'doc')
    #module_alias(env, 'install-doc', install)
    #add_to_module_alias(env, 'install', 'install-doc')



#   files= ["#/bin/imppy.sh", "#/tools/run_all_tests.py"]+\
#        [x.abspath for x in Glob("test_*.py")+ Glob("*/test_*.py")]

def IMPModuleTest(env):
    """Pseudo-builder to run tests for an IMP module. The single target is
       generally a simple output file, e.g. 'test.passed', while the single
       source is a Python script to run (usually run-all-tests.py).
       Right now, the assumption is made that run-all-tests.py executes
       all files called test_*.py in the current directory and subdirectories.
       If the TEST_ENVSCRIPT construction variable is set, it is a shell
       script to run to set up the environment to run the test script.
       A convenience alias for the tests is added, and they are always run."""
    files= ["#/tools/imppy.sh", "#/scons_tools/run-all-tests.py"]+\
        [x.abspath for x in module_glob(["test_*.py", "*/test_*.py"])]
    files.append(env.Alias(env['IMP_MODULE']+"-python"))
    #print files
    test = env._IMPModuleTest(target="test.passed", source=files)
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
            def fail(env, target, source):
                print "The library " +l +" is required by module but could "\
                    + "not be linked."
                return 1
            env.invalidate(fail)
            return
    for h in headers:
        r= eval("conf.Check"+hname(h)+"()")
        if not r:
            def fail(env, target, source):
                print "The header "+ h +" is required by module but could "\
                    + "not be found."
                return 1
            env.invalidate(fail)
            return

def invalidate(env, fail_action):
    """'Break' an environment, so that any builds with it use the fail_action
       function (which should be an Action which terminates the build)"""
    for var in ('SHLINKCOM', 'CCCOM', 'CXXCOM', 'SHCCCOM', 'SHCXXCOM',
                'SWIGCOM'):
        env[var] = fail_action
    #env.Append(BUILDERS={'_IMPModuleTest': Builder(action=fail_action)})
    env['VALIDATED'] = False

def validate(env):
    """Confirm that a module's environment is OK for builds."""
    module = env['IMP_MODULE']
    env['VALIDATED'] = True

def IMPModuleBuild(env, version, required_modules=[],
                   optional_dependencies=[], config_macros=[],
                   module=None, cpp=True, module_suffix=None,
                   module_include_path=None, module_src_path=None, module_preproc=None,
                   module_namespace=None, module_nicename=None,
                   required_dependencies=[],
                   required_libraries=[], required_headers=[]):
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
    # Check required modules and add kernel
    if module != 'kernel':
        for x in required_modules:
            if x.startswith("imp_"):
                print "Required modules should have the name of the module (eg 'algebra'), not the name of the library."
                print required_modules
                raise ValueError(x)
            if x=='kernel':
                print "You do not need to list the kernel as a required module"
                print required_modules
                raise ValueError(x)
        #required_modules.append('kernel')
    else:
        required_modules=[]
    #print module_suffix
    #print module_src_path
    #print module_include_path
    #print module_preproc
    #print module_namespace
    m_libs=[]
    for d in optional_dependencies:
        if d== "CGAL":
            if env['CGAL_LIBS']:
                m_libs=m_libs+env['CGAL_LIBS']
        elif d== "BOOST_FILESYSTEM":
            if env['BOOST_LIBS']:
                m_libs=m_libs+env['BOOST_FILESYSTEM_LIBS']
        else:
            raise ValueError("Do not understand optional dependency: " +d)
    env[module+"_libs"]=m_libs
    env[module+"_required_modules"]=required_modules
    env[module+"_optional_dependencies"]= optional_dependencies
    env['IMP_MODULES_ALL'].append(module)


    env.Prepend(SCANNERS = [swig.scanner, swig.inscanner])

    env = env.Clone()
    env['IMP_REQUIRED_MODULES']= required_modules

    env.Append(BUILDERS = {'IMPModuleConfigH': config_h.ConfigH,
                           'IMPModuleConfigCPP': config_h.ConfigCPP,
                           'IMPModuleLinkTest': link_test.LinkTest,
                           'IMPGeneratePCH': pch.GeneratePCH,
                           'IMPBuildPCH': pch.BuildPCH})
    env['IMP_MODULE'] = module
    env['IMP_MODULE_SUFFIX'] = module_suffix
    env['IMP_MODULE_INCLUDE_PATH'] = module_include_path
    env['IMP_MODULE_SRC_PATH'] = module_src_path
    env['IMP_MODULE_PREPROC'] = module_preproc
    env['IMP_MODULE_NAMESPACE'] = module_namespace
    env['IMP_MODULE_NICENAME'] = module_nicename
    #env['IMP_MODULE_VERSION'] = "SVN"
    env['IMP_MODULE_AUTHOR'] = "A. Biologist"
    env['IMP_MODULE_CPP']= cpp
    env.Prepend(CPPPATH=['#/build/include'])
    env.Prepend(LIBPATH=['#/build/lib'])

    if cpp:
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
    env.Append(BUILDERS={'_IMPColorizePython': examples.ColorizePython})
    env.Append(BUILDERS={'_IMPExamplesDox': examples.MakeDox})
    env.Append(BUILDERS={'_IMPSWIG': swig.SwigIt})
    env.Append(BUILDERS={'_IMPPatchSWIG': swig.PatchSwig})
    env.Append(BUILDERS={'_IMPSWIGPreface': swig.SwigPreface})
    env.Append(BUILDERS={'_IMPMakeModPage': modpage.MakeModPage})
    env.Append(BUILDERS={'IMPRun': run.Run})
    env.AddMethod(validate)
    env.AddMethod(invalidate)
    env['TEST_ENVSCRIPT'] = None
    env['VALIDATED'] = None

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
            print "Could not run svnversion: %s" % str(detail)
    env['IMP_MODULE_VERSION'] = version

    if not env.GetOption('clean') and not env.GetOption('help'):
        if len(required_libraries)+len(required_headers) > 0:
            check_libraries_and_headers(env, required_libraries, required_headers)
        for x in required_dependencies:
            if x== "modeller":
                if not env.get('HAS_MODELLER', False):
                    print "  (modeller missing, disabled)"
                    env.invalidate(modeller_test.fail)
            else:
                raise ValueError("Do not know dependency "+x)

    if env['VALIDATED'] is not None:
        print "IMP."+env['IMP_MODULE']+" is disabled"
        Return()
    else:
        print "Configuring module IMP." + env['IMP_MODULE']+" version "+env['IMP_MODULE_VERSION'],
    if not env['IMP_MODULE_CPP']:
        print " (python only)",
    print



    nice_deps = expand_dependencies(env,required_modules)
    #print "nice is "+str(nice_deps)
    all_deps=["IMP."+x for x in nice_deps if x is not "kernel"]+required_libraries
    if len(all_deps) > 0:
        nice_deps.remove('kernel')
        print "  (requires " +", ".join(all_deps) +")"

    env['IMP_MODULE_CONFIG']=config_macros

    vars=make_vars(env)
    env.validate()
    env.SConscript('doc/SConscript', exports='env')
    env.SConscript('examples/SConscript', exports='env')
    env.SConscript('data/SConscript', exports='env')

    if env['IMP_MODULE_CPP']:
        env.SConscript('include/SConscript', exports='env')
        env.SConscript('src/SConscript', exports='env')
        env.SConscript('bin/SConscript', exports='env')
    if env['python']:
        env.SConscript('pyext/SConscript', exports='env')
        env.SConscript('test/SConscript', exports='env')

    add_to_global_alias(env, 'install', 'install')
    return env
