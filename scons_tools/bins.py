"""Various utilities for handling binaries"""

from SCons.Script import Builder, File, Action, Glob, Return, Alias, Dir, Move, Copy, Scanner
import data
import os
import os.path
import environment as sce
import install as sci
import paths as scp
import data as scd



def _action_run(target, source, env):
    # yeah, should do something fancy in python
    ret= env.Execute(source[0].abspath+" "+source[1].abspath +" > "+\
                     target[0].abspath)
    return ret

def _print(target, source, env):
    print "Running benchmark "+str(source[1].path)


def _get_run_benchmark(env):
    comstr="%sRunning benchmark %s$SOURCE%s"%(env['IMP_COLORS']['purple'],
                                              env['IMP_COLORS']['end'],
                                              env['IMP_COLORS']['end'])
    return Builder(action=Action(_action_run,
                                 _print,
                                 comstr=comstr),
                   comstr=comstr)


def _handle_cpp_bin(env, f, output_dir):
    # strip suffix
    oname= scp.get_output_path(env, f, output_dir)[:-4]
    #print "binary", f.abspath, oname
    ret=env.Program(source=f, target=oname)
    return ret[0]

def _handle_py_bin(env, f, output_dir):
    opath=scp.get_output_path(env, f, output_dir)
    return opath

def _handle_benchmark(env, f):
    runit=_get_run_benchmark(env)(env, target=File(f.abspath+".results"),
                                            source=[File("#/tools/imppy.sh"), f])
    env.Alias("benchmarks", runit)
    return runit

def handle_bins(env, input_files, output_dir, extra_modules=[]):
    env= sce.get_bin_environment(env, extra_modules=extra_modules)
    retb=[]
    for f in input_files:
        if str(f).endswith(".cpp"):
            cur=_handle_cpp_bin(env, f, output_dir)
        else:
            cur=_handle_py_bin(env, f, output_dir)
        retb.append(cur)
    return retb

def handle_benchmarks(ienv, input_files, output_dir, extra_modules=[]):
    env= sce.get_benchmark_environment(ienv,
                                       extra_modules=extra_modules)
    if len(input_files)==0:
        return ([], [])
    if not env:
        msg="%sBenchmarks disabled as the benchmark module is missing%s"
        print msg%(ienv['IMP_COLORS']['red'], ienv['IMP_COLORS']['end'])
        return ([], [])
    retb=[]
    retr=[]
    for f in input_files:
        if str(f).endswith(".cpp"):
            cur=_handle_cpp_bin(env, f, output_dir)
        else:
            cur=_handle_py_bin(env, f, output_dir)
        #print "benchmark", str(f)
        retb.append(cur)
        retr.append(_handle_benchmark(env, cur))
    return (retb, retr)
