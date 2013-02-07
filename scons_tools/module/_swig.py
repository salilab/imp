from SCons.Script import Glob, Dir, File, Builder, Action, Exit, Scanner, Move
import SCons
import os
import sys
import re
import scons_tools.dependency
import scons_tools.module
import scons_tools.data

# standard include files
base_includes= ["IMP_base.macros.i",
                "IMP_base.exceptions.i",
                "IMP_base.directors.i",
                "IMP_base.types.i",
                "IMP_base.refcount.i",
                "IMP_base.streams.i",
                "IMP_base.streams_kernel.i"]
kernel_includes= ["IMP_kernel.macros.i"]


def _null_scanner(node, env, path):
    #print "null scanning", node.abspath
    return []




def _action_simple_swig(target, source, env):
    module= str(source[0])
    swig= str(source[1])
    swigpath= str(source[2])
    includepath= str(source[3])
    input= source[2] # ignored
    ret=env.Execute("cd %s; %s --swig=%s --module=%s --swigpath=%s --includepath=%s"
    %(Dir("#/build").abspath,
      File("#/scons_tools/build_tools/make_swig_wrapper.py").abspath,
      swig,
      module,
      swigpath,
        includepath))
    print "Done running swig", str(source[0])
    return ret

def _print_simple_swig(target, source, env):
    print "Running swig on "+str(source[0])

def _action_swig_scanner(node, env, path):
    realpath= node.abspath
    print realpath
    name= realpath[realpath.rfind("/")+5:-2]
    print name
    env.Execute("cd %s; %s --name=%s --swig=%s --swigpath=%s --includepath=%s --build_system=scons"%(Dir("#/build").abspath,
                                                 File("#/scons_tools/build_tools/make_swig_deps.py").abspath,
                                                 name,
                                                 env.get("swigprogram", "swig"),
        env.get("swigpath", ""), env.get("includepath", "")))

    lines= open(File("#build/src/%s_swig.deps"%name).abspath, "r").readlines()
    deps=[File(l.strip()) for l in lines]
    return deps

scanner= Scanner(function=_action_swig_scanner, skeys=['.i'], name="swigscanner", recursive=False)

def get_swig_action(env):
    comstr="%sRunning swig on %s$SOURCE%s"%(env['IMP_COLORS']['purple'],
                                            env['IMP_COLORS']['end'],
                                            env['IMP_COLORS']['end'])
    return Builder(action=Action(_action_simple_swig,
                                 _print_simple_swig,
                                 comstr=comstr),
                                 source_scanner=scanner,
                                 comstr=comstr)
