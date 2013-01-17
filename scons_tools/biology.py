import utility
import dependency
import doc
import bug_fixes
import scons_tools
import data
import environment
import install
import test
from SCons.Script import Builder, File, Action, Glob, Return, Alias, Dir

def _run_sample(target, source, env):
    print "hi"
    env.Execute("cd "+source[0].abspath + ";"+source[1].abspath+ " python " +source[2].abspath+" 1 1000>/dev/null")

def _print_sample(target, source, env):
    print "sampling", source[2].abspath

def _run_analysis(target, source, env):
    env.Execute("cd "+source[0].abspath + ";"+source[1].abspath+ " python " +source[2].abspath +">/dev/null")

def _print_analysis(target, source, env):
    print "analyzing", source[2].abspath

def IMPSystem(env, name=None, version=None,
              authors=None,
              brief="", overview="",
              publications=None,
              license="standard",
              required_modules=[],
              optional_dependencies=[],
              required_dependencies=[],
              extra_data=[],
              testable=False,
              parallelizable=False,
              last_imp_version="unknown",
              python=True):
    if authors:
        print >> sys.stderr, "You should specify information by editing the overview.dox file."
    if not name:
        name= Dir(".").abspath.split("/")[-1]
    if env.GetOption('help'):
        return
    dirs = scons_tools.paths.get_sconscripts(env)
    local_module=False
    for d in dirs:
        env.SConscript(d, exports=['env'])
    (nenv, version, found_optional_modules, found_optional_dependencies) =\
         utility.configure(env, name, "system", version,
                           required_modules=required_modules,
                           optional_dependencies=optional_dependencies,
                           required_dependencies= required_dependencies)
    if not nenv:
        data.get(env).add_application(name, ok=False)
        return
    else:
        if nenv["IMP_PASS"] != "RUN":
            return

        lkname="system_"+name.replace(" ", "_").replace(":", "_")
        pre="\page "+lkname+" "+name
        extrasections=[]
        if testable:
            extrasections.append(("Testable", "Yes"))
        else:
            extrasections.append(("Testable", "No"))
        if parallelizable:
            extrasections.append(("Parallelizeable", "Yes"))
        else:
            extrasections.append(("Parallelizeable", "No"))
        if last_imp_version != "unknown":
            extrasections.append(("Last known good \imp version", last_imp_version))
        else:
            vtxt=  "\n\\section lkgversion Last known good IMP version\n"+\
                "unknown"+"\n"
        data.get(env).add_system(name, link="\\ref "+lkname+' "'+name+'"',
                                 dependencies=required_dependencies\
                                     +found_optional_dependencies,
                                 unfound_dependencies=[x for x in optional_dependencies
                                                       if not x in
                                                       found_optional_dependencies],
                                 modules= required_modules+found_optional_modules,
                                 version=version)
        for d in dirs:
            if str(d).split("/")[0] != "local":
                env.SConscript(d, exports=['env'])
        scons_tools.data.get(env).add_to_alias("all", env.Alias(name))
        env= nenv
        for m in required_modules+found_optional_modules:
            env.Depends(scons_tools.data.get(env).get_alias(name),
                         scons_tools.data.get(env).get_alias(m))
        if testable:
            samples= Glob("sample_[0123456789]*.py")
            samples.sort(utility.file_compare)
            analysis= Glob("analyze_[0123456789]*.py")
            analysis.sort(utility.file_compare)
            tt= []
            tests = test.add_tests(env, samples+analysis, "system")
            for t in tests:
                env.Depends(t, scons_tools.data.get(env).get_alias(name))

        # link files in build dir
        allfiles= []
        for suff in ["*.txt", "*.mrc", "*.pdb", ".py", ".param", ".input", ".lib"]:
            allfiles.extend(Glob("*/*"+suff))
            allfiles.extend(Glob("*"+suff))
        for f in allfiles+extra_data:
            inst=install.install(env, "biological_systems/"+name+"/", f)
            scons_tools.data.get(env).add_to_alias(env.Alias(name), inst)
            #if f.path.endswith(".py"):
            #     example.add_python_example(env, f, f.path)
            #env.AlwaysBuild(install)
        return env
