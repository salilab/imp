import utility
import dependency
import doc
import bug_fixes
import scons_tools
from SCons.Script import Builder, File, Action, Glob, Return, Alias, Dir

def _get_system_name(env):
    return env['IMP_SYSTEM_NAME']

def IMPSystem(env, name, version,
              authors,
              brief, overview,
              publications=None,
              license="standard",
              required_modules=[],
              optional_dependencies=[],
              required_dependencies=[]):
    if env.GetOption('help'):
        return
    (ok, version, found_optional_modules, found_optional_dependencies) =\
         utility.configure(env, name, "application", version,
                           required_modules=required_modules,
                           optional_dependencies=optional_dependencies,
                           required_dependencies= required_dependencies)
    if not ok:
        return
    doc.add_doc_page(env, "\page page_"+name+" "+name,
                                 authors, version,
                                 brief, overview,
                                 publications,
                                 license)
    env= bug_fixes.clone_env(env)
    env['IMP_SYSTEM_NAME']=name
    utility.add_link_flags(env, required_modules,
                           required_dependencies+env.get_found_dependencies(optional_dependencies))
    return env
