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
    input= source[2] # ignored
    ret=env.Execute("cd %s; %s --swig=%s --module=%s --swigpath=%s"
    %(Dir("#/build").abspath,
      File("#/scons_tools/build_tools/make_swig_wrapper.py").abspath,
      swig,
      module,
      swigpath))
    return ret

def _print_simple_swig(target, source, env):
    print "Running swig on "+str(source[0])


def swig_scanner(node, env, path):
    import re
    contents= node.get_contents()
    # scons recurses with the same scanner, rather than the right one
    # print "Scanning "+str(node)
    dta= scons_tools.data.get(env)
    if str(node).endswith(".h"):
        # we don't care about recursive .hs for running swig
        return []
    else :
        oldret=[]
        ret=[]
        for x in re.findall('\n%include\s"([^"]*.h)"', contents):
            if x.startswith("IMP/"):
                xc= x[4:]
                if xc.find("/") != -1:
                    module= xc[0:xc.find("/")]
                else:
                    module="kernel"
                if module=="internal":
                    module="kernel"
                if not dta.modules[module].external:
                    ret.extend(["#/build/include/"+x])

        for x in re.findall('\n%include\s"IMP_([^"]*).i"', contents)\
                +re.findall('\n%import\s"IMP_([^"]*).i"', contents):
            mn= x.split(".")[0]
            if not dta.modules[mn].external:
                ret.append("#/build/swig/IMP_"+x+".i")
        retset=set(ret)
        ret=list(retset)
        ret.sort()
    return ret

def get_swig_action(env):
    comstr="%sRunning swig on %s$SOURCE%s"%(env['IMP_COLORS']['purple'],
                                            env['IMP_COLORS']['end'],
                                            env['IMP_COLORS']['end'])
    return Builder(action=Action(_action_simple_swig,
                                 _print_simple_swig,
                                 comstr=comstr),
                                 comstr=comstr)
