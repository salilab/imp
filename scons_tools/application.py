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
import build_tools.tools
from SCons.Script import Builder, File, Action, Glob, Return, Alias, Dir
import sys
import os.path

def split(string):
    return [x for x in string.split(":") if x != ""]

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


    if authors:
        print >> sys.stderr, "You should specify information by editing the overview.dox file."

    if env.GetOption('help'):
        return
    name= Dir(".").abspath.split("/")[-1]
    info=scons_tools.build_tools.tools.get_application_info(name,
                                                       env.get("datapath", ""),
                                                       Dir("#/build").abspath)
    if not info["ok"]:
        return None
    env = scons_tools.environment.get_named_environment(env, name,
                                                        info["modules"],
                                                  info["dependencies"])
    scons_tools.data.get(env).add_to_alias("all", env.Alias(name))
    for d in scons_tools.paths.get_sconscripts(env):
        env.SConscript(d, exports=['env'])

    if env["IMP_PASS"] != "BUILD":
        return env
    cppexes= build_tools.tools.get_application_executables(scons_tools.paths.get_input_path(env, "."))
    benv= environment.get_bin_environment(env)
    binclude=[]
    for c in cppexes:
        name= os.path.split(os.path.splitext(c[0][0])[0])[1]
        if c[1] != binclude:
            benv= environment.get_bin_environment(env)
            benv.Append(CPPPATH=c[1])
            binclude=c[1]
        prog= benv.Program(target="#/build/bin/"+name, source=c[0])
        dta= data.get(env)
        dta.add_to_alias(environment.get_current_name(env), prog)
    return env

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
    pass


def IMPApplicationTest(env, python_tests=[]):
    if env["IMP_PASS"] != "RUN":
        return
    files= [File(x).abspath for x in python_tests]
    tests = test.add_tests(env, source=files, type='application unit test')
    for t in tests:
        env.Requires(t, env.Alias(environment.get_current_name(env)))
