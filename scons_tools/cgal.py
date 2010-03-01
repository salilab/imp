"""Simple configure checks for CGAL"""
import checks

def _check(context):
    cgal = context.env['cgal']
    if cgal is False or cgal is 0:
        context.Message('Checking for CGAL ...')
        context.Result("disabled")
        return False

    ret= checks.check_lib(context, lib='CGAL', header=['CGAL/Gmpq.h', 'CGAL/Lazy_exact_nt.h'],
                          body='CGAL_assertion(1); CGAL::Lazy_exact_nt<CGAL::Gmpq> q;',
                          extra_libs=['gmp', 'mpfr', 'm','boost_thread-mt', 'boost_thread', 'pthread'])
    context.Message('Checking for CGAL ...')
    context.Result(ret[0])
    if ret[0]:
        context.env['CGAL_LIBS'] = ret[1]
        #context.env.Append(CPPDEFINES=['IMP_USE_CGAL'])
        if context.env['CC'] == 'gcc':
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
#if BOOST_VERSION > 103700
  #include "boost/math/distributions/students_t.hpp"
#endif


        int main()
        {
            return 0;
        }
        """, '.cpp')
            if rett:
                context.env.Append(CPPDEFINES=['BOOST_HAS_GCC_TR1'])
            context.Result(ret34)

    return ret[0]

def configure_check(env):
    env['CGAL_LIBS'] = ['']
    custom_tests = {'CheckCGAL':_check}
    conf = env.Configure(custom_tests=custom_tests)
    #if not env.GetOption('clean') and not env.GetOption('help'):
    conf.CheckCGAL()
    conf.Finish()
