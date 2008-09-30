"""Simple configure checks for boost"""

import os.path
from SCons.Script import *

def _check(context, version):
    # Boost versions are in format major.minor.subminor
    v_arr = version.split(".")
    version_n = 0
    if len(v_arr) > 0:
        version_n += int(v_arr[0])*100000
    if len(v_arr) > 1:
        version_n += int(v_arr[1])*100
    if len(v_arr) > 2:
        version_n += int(v_arr[2])

    context.Message('Checking for Boost version >= %s... ' % (version))
    # Check some common locations for the boost headers, since there is no
    # pkg-config script:
    for dir in ['', '/opt/local/include', '/sw/include', '/usr/local/include']:
        inc = os.path.join(dir, 'boost/version.hpp')
        ret = context.TryRun("""
        #include "%s"

        int main()
        {
            return BOOST_VERSION >= %d ? 0 : 1;
        }
        """ % (inc, version_n), '.cpp')[0]
        if ret:
            context.env['BOOST_CPPPATH'] = dir
            if dir == '':
                context.Result(ret)
            else:
                context.Result(dir)
            return ret
    context.Result(ret)
    return ret

def configure_check(env, version):
    custom_tests = {'CheckBoost':_check}
    conf = env.Configure(custom_tests=custom_tests)
    env['BOOST_CPPPATH'] = ''
    if not env.GetOption('clean') and not env.GetOption('help') \
       and conf.CheckBoost(version) is 0:
        Exit("Boost version >= %s is required to build IMP" % version)
    conf.Finish()
