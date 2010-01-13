"""Simple configure checks for graphviz"""
import checks

def _check(context):
    has_dot = context.env['dot']
    if has_dot is False:
        context.Message('Checking for graphviz ...')
        context.Result("disabled")
        return False

    ret= checks.check_lib(context, lib='CGAL', header='CGAL/Gmpq.h',
                          body='CGAL_assertion(1); CGAL::Gmpq q;',
                          extra_libs=['gmp', 'm'])
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
                context.env.Append(CCFLAGS=['-frounding-math'])
        context.Result(ret34)
    return ret[0]

def configure_check(env):
    env['CGAL_LIBS'] = ['']
    custom_tests = {'CheckCGAL':_check}
    conf = env.Configure(custom_tests=custom_tests)
    if not env.GetOption('clean') and not env.GetOption('help'):
        conf.CheckCGAL()
    conf.Finish()
