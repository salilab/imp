import utility
import dependency
import doc
import bug_fixes
import scons_tools
from SCons.Script import Builder, File, Action, Glob, Return, Alias, Dir

def _get_application_name(env):
    return env['IMP_APP_NAME']

def IMPApplication(env, name,
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
    print "Configuration application", name
    if len(required_modules+required_dependencies)>0:
        print "  (requires", ", ".join(required_modules+required_dependencies)+")"
    env= bug_fixes.clone_env(env)
    env['IMP_APP_NAME']=name
    utility.add_link_flags(env, required_modules,
                           required_dependencies+env.get_found_dependencies(optional_dependencies))
    dirs = Glob("*/SConscript")
    for d in dirs:
        env.SConscript(d, exports=['env'])
    return env

def IMPCPPBinary(envi, target, source):
    if envi.GetOption('help'):
        return
    env= scons_tools.get_bin_environment(envi)

    prog= env.Program(target="#build/bin/"+target, source=source)
    bindir = env.GetInstallDirectory('bindir')
    install = env.Install(bindir, prog)
    env.Alias(_get_application_name(env), prog)
    env.Alias(_get_application_name(env)+"-install", install)
    env.Alias("all", prog)
    env.Alias("install", install)


def IMPPythonExecutable(env, file):
    if env.GetOption('help'):
        return
    bindir = env.GetInstallDirectory('bindir')
    build= env.Install("#/build/bin", file)
    install = env.Install(bindir, file)
    env.Alias(_get_application_name(env), build)
    env.Alias(_get_application_name(env)+"-install", install)


def IMPApplicationTest(env, python_tests=[]):
    files= ["#/tools/imppy.sh", "#/scons_tools/run-all-tests.py"]+\
        [File(x).abspath for x in python_tests]
    test = env.IMPApplicationRunTest(target="test.passed", source=files,
                                     TEST_TYPE='unit test')
    env.AlwaysBuild("test.passed")
    env.Requires(test, env.Alias(_get_application_name(env)))
    env.Alias('test', test)
