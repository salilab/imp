import scons_tools.utility
import dependency
import scons_tools.doc
import scons_tools
from SCons.Script import Builder, File, Action, Glob, Return, Alias, Dir

def _get_application_name(env):
    return env['IMP_APP_NAME']

def IMPApplicationDoc(env, authors,
                   brief, overview,
                   publications=None,
                   license="standard"):
    scons_tools.doc.add_doc_page(type="\page page_"+_get_application_name(env),
                                 authors=authors,
                                 brief=brief, overview=overview,
                                 publications=publications,
                                 license=license)

def IMPCPPApplication(envi, target, source, required_modules=[],
                      optional_dependencies=[],
                      required_dependencies=[]):
    if envi.GetOption('help'):
        return
    for m in required_modules:
        if not envi.get_module_ok(m):
            print target, "disabled due to missing module IMP."+m
            return
    for d in required_dependencies:
        if not envi.get_dependency_ok(d):
            print target, "disabled due to missing dependency", d
            return
    print "Configuration application", target
    if len(required_modules+required_dependencies)>0:
        print "  (requires", ", ".join(required_modules+required_dependencies)+")"
    env= scons_tools.get_bin_environment(envi)

    scons_tools.utility.add_link_flags(env, required_modules,
                           required_dependencies+env.get_found_dependencies(optional_dependencies))

    prog= env.Program(target=target, source=source)
    bindir = env.GetInstallDirectory('bindir')
    build= env.Install("#/build/bin", prog)
    env['IMP_APPLICATION']=str(target)
    install = env.Install(bindir, prog)

    env.SConscript('test/SConscript', exports=['env'])
    env.Alias(env['IMP_APPLICATION'], build)
    env.Alias(env['IMP_APPLICATION']+"-install", install)
    env.Alias("all", build)
    env.Alias("install", install)
def IMPApplicationTest(env, python_tests=[]):
    files= ["#/tools/imppy.sh", "#/scons_tools/run-all-tests.py"]+\
        [File(x).abspath for x in python_tests]
    test = env.IMPApplicationRunTest(target="test.passed", source=files,
                                     TEST_TYPE='unit test')
    env.AlwaysBuild("test.passed")
    env.Requires(test, env.Alias(env['IMP_APPLICATION']))
    env.Alias('test', test)
