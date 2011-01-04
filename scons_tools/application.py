import utility
import dependency
import doc
import bug_fixes
import install
import test
import data
import environment
import scons_tools
from SCons.Script import Builder, File, Action, Glob, Return, Alias, Dir


def IMPApplication(env, name, version,
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
    (nenv, version, found_optional_modules, found_optional_dependencies) =\
         utility.configure(env, name, "application", version,
                           required_modules=required_modules,
                           optional_dependencies=optional_dependencies,
                           required_dependencies= required_dependencies)
    if not nenv:
        data.get(env).add_application(name, ok=False)
        return
    else:
        if python:
            pm=required_modules+found_optional_modules
        else:
            pm=[]
        lkname="application_"+name.replace(" ", "_").replace(":", "_")
        pre="\page "+lkname+" "+name
        doc.add_doc_page(env, pre,
                         authors, version,
                         brief, overview,
                         publications,
                         license)
        data.get(env).add_application(name, link="\\ref "+lkname+' "'+name+'"',
                                      dependencies=required_dependencies\
                                          +found_optional_dependencies,
                                      unfound_dependencies=[x for x in optional_dependencies
                                                            if not x in
                                                            found_optional_dependencies],
                                      modules= required_modules+found_optional_modules,
                                      python_modules=pm,
                                      version=version)


    env= environment.get_bin_environment(nenv)
    scons_tools.data.get(env).add_to_alias("all", env.Alias(name))
    dirs = Glob("*/SConscript")
    for d in dirs:
        env.SConscript(d, exports=['env'])
    return env

def IMPCPPExecutable(envi, target, source):
    if envi.GetOption('help'):
        return
    env= environment.get_bin_environment(envi)

    prog= env.Program(target="#/build/bin/"+target, source=source)
    bindir = install.install(env,'bindir', prog[0])


def IMPPythonExecutable(env, file):
    if env.GetOption('help'):
        return
    install.install(env, "bindir", file)


def IMPApplicationTest(env, python_tests=[]):
    files= ["#/tools/imppy.sh", "#/scons_tools/run-all-tests.py"]+\
        [File(x).abspath for x in python_tests]
    tst=test.add_test(env, source=files,
                  type='unit test')
    env.Requires(tst, env.Alias(environment.get_current_name(env)))
