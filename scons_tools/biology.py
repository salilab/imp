import utility
import dependency
import doc
import bug_fixes
import scons_tools
import data
import environment
from SCons.Script import Builder, File, Action, Glob, Return, Alias, Dir

def IMPSystem(env, name, version="",
              authors=[],
              brief="", overview="",
              publications=None,
              license="standard",
              required_modules=[],
              optional_dependencies=[],
              required_dependencies=[],
              runable=False,
              last_imp_version="unknown",
              python=True):
    if env.GetOption('help'):
        return
    (ok, version, found_optional_modules, found_optional_dependencies) =\
         utility.configure(env, name, "application", version,
                           required_modules=required_modules,
                           optional_dependencies=optional_dependencies,
                           required_dependencies= required_dependencies)
    if not ok:
        data.get(env).add_application(name, ok=ok)
        return
    else:
        if python:
            pm=required_modules+found_optional_modules
        else:
            pm=[]
        lkname="system_"+name.replace(" ", "_").replace(":", "_")
        pre="\page "+lkname+" "+name
        if runable:
            rtxt= "\n\\section runable Runable\nYes.\n"
        else:
            rtxt= "\n\\section runable Runable\nNo.\n"
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
        dirs = Glob("*/SConscript")
        for d in dirs:
            env.SConscript(d, exports=['env'])

        env= scons_tools.environment.get_named_environment(env, name)
        utility.add_link_flags(env, required_modules,
                               required_dependencies+found_optional_dependencies)
        return env


def IMPSystemHelperModuleBuild(env):
    dta= data.get(env)
    aname= scons_tools.environment.get_current_name(env)
    env.IMPModuleBuild(version=dta.systems[aname].version,
                       required_modules= dta.systems[aname].modules,
                       required_dependencies= dta.systems[aname].dependencies,
                       module=aname+"_helper",
                       helper_module=True)
