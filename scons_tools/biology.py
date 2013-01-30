import utility
import dependency
import doc
import bug_fixes
import scons_tools
import data
import environment
import install
import test
from SCons.Script import Builder, File, Action, Glob, Return, Alias, Dir

def _run_sample(target, source, env):
    print "hi"
    env.Execute("cd "+source[0].abspath + ";"+source[1].abspath+ " python " +source[2].abspath+" 1 1000>/dev/null")

def _print_sample(target, source, env):
    print "sampling", source[2].abspath

def _run_analysis(target, source, env):
    env.Execute("cd "+source[0].abspath + ";"+source[1].abspath+ " python " +source[2].abspath +">/dev/null")

def _print_analysis(target, source, env):
    print "analyzing", source[2].abspath

def IMPSystem(env, name=None, version=None,
              authors=None,
              brief="", overview="",
              publications=None,
              license="standard",
              required_modules=[],
              optional_dependencies=[],
              required_dependencies=[],
              extra_data=[],
              testable=False,
              parallelizable=False,
              last_imp_version="unknown",
              python=True):
    if authors:
        print >> sys.stderr, "You should specify information by editing the overview.dox file."
    return env
