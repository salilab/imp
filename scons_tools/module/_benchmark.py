from SCons.Script import Glob, Dir, File, Builder, Action, Exit, Scanner, Move
import SCons
import os
import sys
import re
import scons_tools.dependency
import scons_tools.module
import scons_tools.data


def _action_run(target, source, env):
    # yeah, should do something fancy in python
    ret= env.Execute(source[0].abspath+" "+source[1].abspath +"| tee "+ target[0].abspath)
    return ret

def _print(target, source, env):
    print "Running benchmark "+str(source[0].path)


def get_run_benchmark(env):
    comstr="%sRunning benchmark %s$SOURCE%s"%(env['IMP_COLORS']['purple'],
                                              env['IMP_COLORS']['end'],
                                              env['IMP_COLORS']['end'])
    return Builder(action=Action(_action_run,
                                 _print,
                                 comstr=comstr),
                   comstr=comstr)


def _action_py_run(target, source, env):
    # yeah, should do something fancy in python
    ret= env.Execute(source[0].abspath+" python "+source[1].abspath +"| tee "+ target[0].abspath)
    return ret

def _print_py(target, source, env):
    print "Running benchmark "+str(source[0].path)


def get_run_py_benchmark(env):
    comstr="%sRunning benchmark %s$SOURCE%s"%(env['IMP_COLORS']['purple'],
                                              env['IMP_COLORS']['end'],
                                              env['IMP_COLORS']['end'])
    return Builder(action=Action(_action_py_run,
                                 _print_py,
                                 comstr=comstr),
                   comstr=comstr)
