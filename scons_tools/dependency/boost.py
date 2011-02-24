"""Simple configure checks for boost"""

from SCons.Script import Exit
import gcc
import scons_tools.data
import scons_tools.utility

def _check(context):
    context.Message('Checking for Boost lib version ... ')
    ret = context.TryRun("""#include <boost/version.hpp>
#include <iostream>

        int main()
        {
            std::cout << BOOST_LIB_VERSION <<std::endl;
            return 0;
        }
        """, '.cpp')
    context.Result(ret[1].replace("_", ".").split('\n')[0])
    if ret[0]:
        try:
            context.env['BOOST_LIB_VERSION']= ret[1].split('\n')[0]
        except:
            print "Bad boost version", repr(ret)
            env.Exit(1)
    else:
        context.env['BOOST_LIB_VERSION']=None
    return ret[0]

def _checks(context):
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

def find_lib_version(env):
    custom_tests = {'CheckBoost':_check}
    conf = env.Configure(custom_tests=custom_tests)
    conf.CheckBoost()
    conf.Finish()
    if env.get('boostlibsuffix', "auto")!="auto":
        env['BOOST_LIBSUFFIX']=env['boostlibsuffix']
    else:
        if env.get('boostversion', None):
            scons_tools.utility.report_error(env, "You must specify the boostlibsuffix if you specify the boostversion")
        custom_tests = {'CheckBoostS':_checks}
        conf = env.Configure(custom_tests=custom_tests)
        conf.CheckBoostS()
        conf.Finish()
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


def get_boost_lib_name(env, name):
    return "boost_"+name+env.get("BOOST_LIBSUFFIX", "")

def add_boost_library(env, nicename, libname, header_name, body=[], extra_boost_libs=[]):
    real_libname=scons_tools.dependency.boost.get_boost_lib_name(env,libname)
    real_dep_names=[]
    for d in extra_boost_libs:
        real_dep_names.append(scons_tools.dependency.boost.get_boost_lib_name(env,d))
    lname="Boost."+nicename
    scons_tools.dependency.add_external_library(env, lname, [real_libname],
                                                header_name,
                                                body=body,
                                                extra_libs=real_dep_names)
