"""Simple configure checks for boost"""

from SCons.Script import Exit
import gcc
import scons_tools.utility

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
    ret = context.TryRun("""#include <boost/version.hpp>
#include <iostream>

        int main()
        {
            std::cout << BOOST_LIB_VERSION <<std::endl;
            std::cout << BOOST_VERSION << std::endl;
            return BOOST_VERSION >= %d ? 0 : 1;
        }
        """ % version_n, '.cpp')
    context.Result(ret[1].replace("_", ".").split('\n')[0])
    if ret[0]:
        try:
            context.env['BOOST_VERSION']= ret[1].split('\n')[1]
            context.env['BOOST_LIB_VERSION']= ret[1].split('\n')[0]
        except:
            print "Bad boost version", repr(ret)
            env.Exit(1)
    else:
        context.env['BOOST_VERSION']=None
        context.env['BOOST_LIB_VERSION']=None
    return ret[0]

def _checks(context, version):
    version=context.env['BOOST_LIB_VERSION']
    for suffix in ['-mt', '', '-'+version+'-mt', '-'+version]:
        ret= context.sconf.CheckLib('boost_filesystem'+suffix, language="c++", autoadd=False)
        if ret:
            context.Message('Checking for Boost lib suffix... ')
            context.env['BOOST_LIBSUFFIX']=suffix
            context.Result(suffix)
            return True
    context.env['BOOST_LIBSUFFIX']=""
    context.Message('Checking for Boost lib suffix... ')
    context.Result('not found')
    return False


def configure_check(env, version):
    if env.get('boostversion', None):
        env['BOOST_VERSION']=env['boostversion']
    else:
        custom_tests = {'CheckBoost':_check}
        conf = env.Configure(custom_tests=custom_tests)
        conf.CheckBoost(version)
        conf.Finish()
        if not env.get("BOOST_VERSION", None):
            return
    if env.get('boostlibsuffix', "auto")!="auto":
        env['BOOST_LIBSUFFIX']=env['boostlibsuffix']
    else:
        if env.get('boostversion', None):
            scons_tools.utility.report_error(env, "You must specify the boostlibsuffix if you specify the boostversion")
        custom_tests = {'CheckBoostS':_checks}
        conf = env.Configure(custom_tests=custom_tests)
        conf.CheckBoostS(version)
        conf.Finish()
    env.Append(IMP_CONFIGURATION=["boostversion='"+env['BOOST_VERSION']+"'"])
    env.Append(IMP_CONFIGURATION=["boostlibsuffix='"+env.get('BOOST_LIBSUFFIX', '')+"'"])

def _tr1check(context):
    context.Message('Checking if Boost and gcc tr1 coexist ...')
    rett = context.TryCompile("""
#include <tr1/tuple>
#include <boost/tuple/tuple.hpp>
    int main()
    {
    return 0;
    }
    """, '.cpp')
    context.Result(rett)
    return rett


def configure_tr1_check(env):
    custom_tests = {'CheckTR1':_tr1check}
    conf = env.Configure(custom_tests=custom_tests)
    if not conf.CheckTR1():
        env['IMP_HAS_BOOST_TR1_BUG']=True
    else:
        pass
    conf.Finish()
