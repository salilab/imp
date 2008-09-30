"""Simple configure checks for EMBED"""

import os.path
from SCons.Script import *

def _check(context):
    """Find EMBED include and library directories"""
    context.Message('Checking for EMBED...')
    em = context.env['embed']
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
    custom_tests = {'CheckEMBED':_check}
    conf = env.Configure(custom_tests=custom_tests)
    if env.GetOption('clean') or env.GetOption('help') \
       or conf.CheckEMBED() is not True:
        for suff in ('CPPPATH', 'LIBPATH', 'LIBS', 'EMPY'):
            env['EM_' + suff] = ""
    conf.Finish()

def fail(env, target, source):
    print """
  ERROR: EMBED and Modeller are required to build %s.

  Use the embed and modeller command line options (or options file) to set the
  directories where EMBED and Modeller are installed (run 'scons -h' for help.)
""" % target[0]
    return 1
