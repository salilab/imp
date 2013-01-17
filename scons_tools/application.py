import utility
import pyscanner
import dependency
import doc
import bug_fixes
import install
import test
import data
import environment
import scons_tools
from SCons.Script import Builder, File, Action, Glob, Return, Alias, Dir
import sys

def IMPApplication(env,
                   authors=None,
                   brief=None, overview=None,
                   version=None,
                   publications=None,
                   license="standard",
                   required_modules=[],
                   optional_dependencies=[],
                   required_dependencies=[],
                   python=True):
    if len(required_modules) >0:
        print >> sys.stderr, "You should use the \"description\" file to describe a application dependencies instead of the SConscript (and remove the variables from the SConscript). One has been created."
        file=open(scons_tools.paths.get_input_path(env, "description"), "w")
        file.write("required_modules="+str(required_modules)+"\n")
        file.write("required_dependencies="+str(required_dependencies)+"\n")
        file.write("optional_dependencies="+str(optional_dependencies)+"\n")
    else:
        exec open(scons_tools.paths.get_input_path(env, "description"), "r").read()

    if authors:
        print >> sys.stderr, "You should specify information by editing the overview.dox file."

    if env.GetOption('help'):
        return
    name= Dir(".").abspath.split("/")[-1]
    nenv=\
        utility.configure_application(env, name, None, version,
                           required_modules=required_modules,
                           optional_dependencies=optional_dependencies,
                           required_dependencies= required_dependencies)
    if nenv:
        env= environment.get_bin_environment(nenv)
        scons_tools.data.get(env).add_to_alias("all", env.Alias(name))
        for d in scons_tools.paths.get_sconscripts(env):
            env.SConscript(d, exports=['env'])
        return env
    else:
        return None

def IMPCPPExecutable(env, target, source):
    if env["IMP_PASS"] != "RUN":
        return
    if env.GetOption('help'):
        return

    prog= env.Program(target="#/build/bin/"+target, source=source)
    dta= data.get(env)
    dta.add_to_alias(environment.get_current_name(env), prog)

def IMPCPPExecutables(env, lst):
    if env["IMP_PASS"] != "RUN":
        return
    if env.GetOption('help'):
        return
    dta= data.get(env)
    for l in lst:
        prog= env.Program(target="#/build/bin/"+l[0], source=l[1])
        dta.add_to_alias(environment.get_current_name(env), prog)

def IMPPythonExecutable(env, file):
    if env["IMP_PASS"] != "RUN":
        return
    def dummy(target, source, env): pass
    _PythonExeDependency = Builder(action=Action(dummy, dummy),
                                   source_scanner=pyscanner.PythonScanner)

    if env.GetOption('help'):
        return
    dta= data.get(env)
    inst = install.install(env, "bindir", file)

    # Make sure that when we install a Python executable we first build
    # any Python modules it uses (env.Install() does not appear to accept
    # source_scanner, so we use a dummy do-nothing builder to add these
    # dependencies)
    pydep = _PythonExeDependency(env, target=None, source=file)
    env.Depends(inst, pydep)
    env.Append(IMP_PYTHON_EXECUTABLES=[file])
    dta.add_to_alias(environment.get_current_name(env), pydep)


def IMPApplicationTest(env, python_tests=[]):
    if env["IMP_PASS"] != "RUN":
        return
    files= [File(x).abspath for x in python_tests]
    tests = test.add_tests(env, source=files, type='application unit test')
    for t in tests:
        env.Requires(t, env.Alias(environment.get_current_name(env)))
