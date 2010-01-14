
import imp_module
from SCons.Script import Glob, Dir, File, Builder, Action, Exit
import os
import sys
import re


def _action_generate_pch_h(target, source, env):
    h = file(target[0].abspath, 'w')
    #print "Generating "+str(h)
    print >> h, """
#include <Python.h>
#include <iostream>
#include <map>
#include <string>
#include <set>
#include <vector>
#include <list>
#include <fstream>
#include <IMP.h>
"""

def _print_generate_pch_h(target, source, env):
    print "Generating pch.h ", str(target[0])

GeneratePCH = Builder(action=Action(_action_generate_pch_h,
                                _print_generate_pch_h))

def _get_string(env, source, target):
    ret=" ".join([env['CXX'], "-o "+target[0].abspath, "-x c++-header",
                  source[0].abspath]+ env['CPPFLAGS']
                 + ["-I"+Dir(x).path for x in env['CPPPATH']] + env['CXXFLAGS']
                 + ["-D"+x for x in env['CPPDEFINES']])
    return ret

def _action_build_pch_h(target, source, env):
    env.Execute(_get_string(env,source, target))

def _print_build_pch_h(target,source,env):
    print _get_string(env, source, target)

BuildPCH = Builder(action=Action(_action_build_pch_h, _print_build_pch_h))
