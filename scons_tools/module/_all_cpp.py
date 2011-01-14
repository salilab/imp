import scons_tools.module
from SCons.Script import Glob, Dir, File, Builder, Action, Exit
import os
import sys
import re
import scons_tools.environment


def _action_all_cpp(target, source, env):
    f= open(target[0].abspath, "w")
    for fl in source:
        pth= fl.abspath
        print >> f, "#include \""+pth+"\""

def _print_all_cpp(target, source, env):
    vars= scons_tools.module._get_module_variables(env)
    print "Generating %(module)s_all.cpp"%vars

_All = Builder(action=Action(_action_all_cpp,
                                 _print_all_cpp))


def get(env, files):
    target=File("#/build/src/"+scons_tools.environment.get_current_name(env)+"_all.cpp")
    return _All(env=env, target=target, source=files)
