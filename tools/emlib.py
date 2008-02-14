"""Simple configure checks for emlib"""

import os.path
from SCons.Script import *

def _check(context):
    """Find EMLIB include and library directories"""
    context.Message('Checking for EMLIB...')
    em = context.env['em']
    if em is False or em is 0:
        context.Result("not found")
        return False

    srcpath = os.path.join(em, 'src', 'emlib')
    if not os.path.exists(srcpath):
        context.Result("not found in " + srcpath)
        return False
    context.env['EM_CPPPATH'] = srcpath
    context.env['EM_LIBPATH'] = srcpath
    context.env['EM_LIBS'] = ['em']
    context.env['EM_EMPY'] = os.path.join(em, 'bin', 'empy.sh')
    context.Result(em)
    return True

def configure_check(env):
    custom_tests = {'CheckEMLib':_check}
    conf = env.Configure(custom_tests=custom_tests)
    if env.GetOption('clean') or env['OPTION_HELP'] \
       or conf.CheckEMLib() is not True:
        for suff in ('CPPPATH', 'LIBPATH', 'LIBS', 'EMPY'):
            env['EM_' + suff] = ""
    conf.Finish()

def fail(env, target, source):
    print """
  ERROR: emlib and Modeller are required to build %s.

  Use the em and modeller command line options (or options file) to set the
  directories where emlib and Modeller are installed (run 'scons -h' for help.)
""" % target[0]
    return 1
