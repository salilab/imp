from SCons.Script import Glob, Dir, File, Builder, Action, Exit, Scanner, Move
import SCons
import os
import sys
import re

def _check(context):
    needversion = [1,3,40]
    needversion_str = ".".join([str(x) for x in needversion])
    failmsg = """
SWIG version %s or later must be installed to support Python, but
%s.
Please make sure 'swig' is found in the path passed to scons.

In particular, if you have SWIG installed in a non-standard location,
please use the 'path' option to add this location to the search path.
For example, if you have SWIG installed in /opt/local/bin/, edit (or create)
config.py and add the line

path='/opt/local/bin'

Since SWIG could not be found, proceeding to build IMP without Python support.

"""
    context.Message('Checking for SWIG version >= %s... ' % needversion_str)
    try:
        version = context.env['SWIGVERSION']
    except:
        ret= os.popen("swig -version").read()
        r= re.compile("SWIG Version ([0-9\.]+)")
        match=r.search(ret)
        if match:
            version= match.groups()[0]
    try:
        v = [int(x) for x in version.split(".")]
    except ValueError:
        context.Result('not found')
        print failmsg % (needversion_str,
                         "it could not be found on your system")
        return False
    if v >= needversion:
        context.Result('%s' % version)
        return True
    else:
        context.Result('no, %s found' % version)
        print failmsg % (needversion_str,
                         "only an older version (%s) " % version + \
                         "was found on your system")
        return False

def configure_check(env):
    custom_tests = {'CheckSWIG':_check}
    conf = env.Configure(custom_tests=custom_tests)
    #if not env.GetOption('clean') and not env.GetOption('help') \
    #   and conf.CheckSWIG() is False:
    if not conf.CheckSWIG():
        env['IMP_PROVIDE_PYTHON']=False
        #env.Append(IMP_BUILD_SUMMARY=["Swig was not found."])
    conf.Finish()
