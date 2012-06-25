import scons_tools.module
from SCons.Script import Glob, Dir, File, Builder, Action, Exit
import os
import sys
import re
import scons_tools.environment
import scons_tools.module
import scons_tools.paths as stp


def _action_all_cpp(target, source, env):
    f= open(target[0].abspath, "w")
    vars= scons_tools.module._get_module_variables(env)
    for fl in source:
        pth= fl.abspath
        print >> f, "#include \""+pth+"\""
    if env['linktest']:
        print >> f, "#include <%(module_include_path)s.h>"%vars

def _print_all_cpp(target, source, env):
    vars= scons_tools.module._get_module_variables(env)
    print "Generating %(module)s_all.cpp"%vars

_All = Builder(action=Action(_action_all_cpp,
                                 _print_all_cpp))


def get(env, files):
    module= scons_tools.module._get_module_name(env)
    target=File(stp.get_build_source_file(env, "all.cpp", module))
    return _All(env=env, target=target, source=files)
