import utility
import dependency
import doc
import bug_fixes
import install
import test
import environment
import scons_tools
from SCons.Script import Builder, File, Action, Glob, Return, Alias, Dir

def _get_application_name(env):
    return env['IMP_APP_NAME']

def IMPApplication(env, name, version,
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
    env= scons_tools.environment.get_named_environment(env, name)
    env['IMP_APP_NAME']=name
    utility.add_link_flags(env, required_modules,
                           required_dependencies+env.get_found_dependencies(optional_dependencies))
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
    test.add_test(env, source=files,
                  type='unit test')
