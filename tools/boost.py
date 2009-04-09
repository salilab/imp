"""Simple configure checks for boost"""

from SCons.Script import Exit

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

        int main()
        {
            return BOOST_VERSION >= %d ? 0 : 1;
        }
        """ % version_n, '.cpp')[0]
    context.Result(ret)
    return ret

def configure_check(env, version):
    custom_tests = {'CheckBoost':_check}
    conf = env.Configure(custom_tests=custom_tests)
    if not env.GetOption('clean') and not env.GetOption('help') \
       and conf.CheckBoost(version) is 0:
        Exit("""
Boost version >= %s is required to build IMP, but it could not be found
on your system. Please see config.log for more details.

In particular, if you have Boost installed in a non-standard location,
please use the 'includepath' option to add this location to the search path.
For example, a Mac using Boost installed with MacPorts will have the Boost
headers in /opt/local/include, so edit (or create) config.py and add the line

includepath='/opt/local/include'
""" % version)
    conf.Finish()
