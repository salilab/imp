import utility
import dependency
import doc
import bug_fixes
import scons_tools
import data
import environment
from SCons.Script import Builder, File, Action, Glob, Return, Alias, Dir

def IMPSystem(env, name, version,
              authors,
              brief, overview,
              publications=None,
              license="standard",
              required_modules=[],
              optional_dependencies=[],
              required_dependencies=[],
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
        doc.add_doc_page(env, "\page system_"+lkname+" "+name,
                                 authors, version,
                                 brief, overview,
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
    env= scons_tools.environment.get_named_environment(env, name)
    utility.add_link_flags(env, required_modules,
                           required_dependencies+found_optional_dependencies)
    return env
