
import imp_module
import SCons
from SCons.Script import Glob, Dir, File, Builder, Action, Exit, Scanner
import os
import sys
import re
import imp_module


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
                  source[0].abspath]+ env.get('CPPFLAGS', [])
                 + ["-I"+Dir(x).path for x in env['CPPPATH']] + env['CXXFLAGS']
                 + ["-D"+x for x in env['CPPDEFINES']])
    return ret

def _action_build_pch_h(target, source, env):
    env.Execute(_get_string(env,source, target))

def _print_build_pch_h(target,source,env):
    print "Building pch "+ str(target)

def _emitter_build_pch(target, source, env):
    SCons.Defaults.SharedObjectEmitter( target, source, env )
    scanner = SCons.Scanner.C.CScanner()
    path = scanner.path(env)
    deps = set([File("#/build/include/IMP.h")])
    # shortcut since the file we are supposed to be scanning may not
    # exist
    toscan=[File("#/build/include/IMP.h")]
    # for some reason that is beyond me, the presence of this line
    # or some such call to scanner is essential to the code below
    # producing the correct result.
    all= [str(x) for x in scanner(File("#/build/include/IMP.h"), env, path)]
    while len(toscan) >0:
        odeps=set(list(deps))
        #print str(toscan[-1])
        # skip python for now
        #print str(toscan[-1])
        #l=[x  for x in scanner(toscan[-1], env, path) if str(x).find('python')==-1]
        l= scanner(toscan[-1], env, path)
        #print [str(x) for x in l]
        #print "list for "+str(toscan[-1]) + " is " +str([str(x) for x in l])
        cdeps= set(l)
        toscan=toscan[:-1]
        ndeps=cdeps-deps
        #print "new are "+str([str(x) for x in ndeps])
        toscan=toscan+list(ndeps)
        deps=cdeps^deps
        #print [str(x) for x in deps]
        #print "queue is " + str([str(x) for x in toscan])
    #print [str(x) for x in source]
    #print [str(x) for x in target]
    #print [str(x) for x in deps]
    sdeps= source+list(deps)
    sdeps.sort(lambda a,b: cmp(str(a), str(b)))
    return (target, sdeps)

BuildPCH = Builder(action=Action(_action_build_pch_h, _print_build_pch_h),
                   emitter=_emitter_build_pch)
