"""Simple configure checks for CGAL"""
import checks

def _check(context):
    context.Message('Checking if we need to disable Boost tr1 ...')
    rett = context.TryCompile("""#include <CGAL/tuple.h>
#include <boost/tr1/tuple.hpp>

        int main()
        {
            return 0;
        }
        """, '.cpp')
    if rett:
        context.env.Append(CPPDEFINES=['BOOST_HAS_GCC_TR1'])
    context.Result(rett)
    return True

def configure_check(env):
    custom_tests = {'CheckCGAL':_check}
    conf = env.Configure(custom_tests=custom_tests)
    #if not env.GetOption('clean') and not env.GetOption('help'):
    if env['CGAL_LIBS'] and env['CXX'] == 'g++':
        conf.CheckCGAL()
    conf.Finish()
