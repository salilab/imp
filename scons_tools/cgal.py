"""Simple configure checks for CGAL"""
import checks

def _check(context):
    if context.env['CGAL_LIBS']:
        #context.env.Append(CPPDEFINES=['IMP_USE_CGAL'])
        if context.env['CXX'] == 'g++':
            context.Message('Checking if CGAL needs -frounding-math ...')
            ret34 = context.TryRun("""#include <CGAL/version.h>
#include <cstdlib>

        int main()
        {
            return CGAL_VERSION_NR >= 1030400000 ? EXIT_SUCCESS : EXIT_FAILURE;
        }
        """, '.cpp')[0]
            if ret34:
                context.env.Append(CXXFLAGS=['-frounding-math'])
            context.Result(ret34)
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
            context.Result(ret34)
    return True

def configure_check(env):
    custom_tests = {'CheckCGAL':_check}
    conf = env.Configure(custom_tests=custom_tests)
    #if not env.GetOption('clean') and not env.GetOption('help'):
    conf.CheckCGAL()
    conf.Finish()
