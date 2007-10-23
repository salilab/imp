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
    context.Result(em)
    return True

def configure_check(env):
    custom_tests = {'CheckEMLib':_check}
    conf = env.Configure(custom_tests=custom_tests)
    if conf.CheckEMLib() is not True:
        for suff in ('CPPPATH', 'LIBPATH', 'LIBS'):
            env['EM_' + suff] = ""
    conf.Finish()

def fail(env, target, source):
    print """
  ERROR: emlib is required to build %s.

  You can set the directory where emlib is installed by using
  'scons em=/opt/em' or similar.
""" % target[0]
    return 1
