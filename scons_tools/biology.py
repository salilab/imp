import utility
import dependency
import doc
import bug_fixes
import scons_tools
from SCons.Script import Builder, File, Action, Glob, Return, Alias, Dir

def _get_system_name(env):
    return env['IMP_SYSTEM_NAME']

def IMPSystem(env, name,
              authors,
              brief, overview,
              publications=None,
              license="standard",
              required_modules=[],
              optional_dependencies=[],
              required_dependencies=[]):
    if env.GetOption('help'):
        return
    doc.add_doc_page(env, "\page page_"+name+" "+name,
                                 authors, "1.0",
                                 brief, overview,
                                 publications,
                                 license)
    for m in required_modules:
        if not env.get_module_ok(m):
            print name, "disabled due to missing module IMP."+m
            return
    for d in required_dependencies:
        if not env.get_dependency_ok(d):
            print name, "disabled due to missing dependency", d
            return
    print "Configuration system", name
    if len(required_modules+required_dependencies)>0:
        print "  (requires", ", ".join(required_modules+required_dependencies)+")"
    env= bug_fixes.clone_env(env)
    env['IMP_SYSTEM_NAME']=name
    utility.add_link_flags(env, required_modules,
                           required_dependencies+env.get_found_dependencies(optional_dependencies))
    return env
