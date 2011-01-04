import utility
import dependency
import doc
import bug_fixes
import scons_tools
import data
import environment
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

def IMPSystem(env, name=None, version="",
              authors=[],
              brief="", overview="",
              publications=None,
              license="standard",
              required_modules=[],
              optional_dependencies=[],
              required_dependencies=[],
              testable=False,
              last_imp_version="unknown",
              python=True):
    if not name:
        name= Dir(".").abspath.split("/")[-1]
    if env.GetOption('help'):
        return
    dirs = Glob("*/SConscript")
    local_module=False
    for d in dirs:
        if str(d).split("/")[0] == "local":
            env.SConscript(d, exports=['env'])
            local_module=True
            required_modules.append(name+"_local")
    (nenv, version, found_optional_modules, found_optional_dependencies) =\
         utility.configure(env, name, "system", version,
                           required_modules=required_modules,
                           optional_dependencies=optional_dependencies,
                           required_dependencies= required_dependencies)
    if not nenv:
        data.get(env).add_application(name, ok=ok)
        return
    else:
        if python:
            pm=required_modules+found_optional_modules
        else:
            pm=[]
        lkname="system_"+name.replace(" ", "_").replace(":", "_")
        pre="\page "+lkname+" "+name
        if testable:
            rtxt= "\n\\section testable Testable\nYes.\n"
        else:
            rtxt= "\n\\section testable Testable\nNo.\n"
        if last_imp_version != "unknown":
            vtxt= "\n\\section lkgversion Last known good IMP version\n"+\
                last_imp_version+"\n"
        else:
            vtxt=  "\n\\section lkgversion Last known good IMP version\n"+\
                "unknown"+"\n"
        doc.add_doc_page(env, "\page "+lkname+" "+name,
                                 authors, version,
                                 brief, overview+rtxt+vtxt,
                                 publications,
                                 license)
        data.get(env).add_system(name, link="\\ref "+lkname+' "'+name+'"',
                                 dependencies=required_dependencies\
                                     +found_optional_dependencies,
                                 unfound_dependencies=[x for x in optional_dependencies
                                                       if not x in
                                                       found_optional_dependencies],
                                 modules= required_modules+found_optional_modules,
                                 python_modules=pm,
                                 version=version)
        for d in dirs:
            if str(d).split("/")[0] != "local":
                env.SConscript(d, exports=['env'])

        env= nenv
        for m in required_modules+found_optional_modules:
            env.Depends(scons_tools.data.get(env).get_alias(name+"-install"),
                         scons_tools.data.get(env).get_alias(m+"-install"))
            env.Depends(scons_tools.data.get(env).get_alias(name),
                         scons_tools.data.get(env).get_alias(m))
        if testable:
            samples= Glob("sample_[0123456789]*.py")
            samples.sort(utility.file_compare)
            analysis= Glob("analyze_[0123456789]*.py")
            analysis.sort(utility.file_compare)
            tt= []
            tst=test.add_test(env, samples+analysis, "system")
            env.Depends(tst,
                        scons_tools.data.get(env).get_alias(name))

        return env


def IMPSystemHelperModuleBuild(env):
    dta= data.get(env)
    aname= scons_tools.environment.get_current_name(env)
    env.IMPModuleBuild(version=dta.systems[aname].version,
                       required_modules= dta.systems[aname].modules,
                       required_dependencies= dta.systems[aname].dependencies,
                       module=aname+"_helper",
                       helper_module=True)
