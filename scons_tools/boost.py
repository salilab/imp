"""Simple configure checks for boost"""

from SCons.Script import Exit
import checks

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
            return BOOST_VERSION >= %d ? 0 : 1;
        }
        """ % version_n, '.cpp')
    context.Result(ret[1].replace("_", ".").split('\n')[0])
    if ret[0]:
        check_libs=[('BOOST_FILESYSTEM_LIBS', ('libboost_filesystem',
                                               'boost/filesystem/path.hpp',
                                               ['libboost_system'])),
                    ('BOOST_PROGRAM_OPTIONS_LIBS', ('libboost_program_options',
                                               'boost/program_options.hpp',
                                               []))]
        for l in check_libs:
            ret1=checks.check_lib(context, lib=l[1][0]+"-mt",
                                  header=l[1][1],
                                  extra_libs=[x+"-mt" for x in l[1][2]])
            if ret1[0]:
                context.env[l[0]]=ret1[1]
            else:
                ret2= checks.check_lib(context, lib=l[1][0], header=l[1][1], extra_libs=l[1][2])
                if ret2[0]:
                    context.env[l[0]]=ret2[1]
                else:
                    context.env['BOOST_LIBS']=False
    if not context.env['BOOST_LIBS']:
        print "WARNING, boost libraries not found, some functionality may be missing."
    return ret[0]

def configure_check(env, version):
    custom_tests = {'CheckBoost':_check}
    conf = env.Configure(custom_tests=custom_tests)
    if env.GetOption('clean') or env.GetOption('help'):
        env['BOOST_LIBS']=False
    else:
        env["BOOST_LIBS"]=True
        if conf.CheckBoost(version) is 0:
            Exit("""
Boost version >= %s is required to build IMP, but it could not be
found on your system.

In particular, if you have Boost installed in a non-standard location,
please use the 'includepath' option to add this location to the search
path.  For example, a Mac using Boost installed with MacPorts will
have the Boost headers in /opt/local/include, so edit (or create)
config.py and add the line

includepath='/opt/local/include'

You can see the produced config.log for more information as to why
boost failed to be found.
""" % version)
    conf.Finish()
