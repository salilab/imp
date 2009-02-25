"""Simple configure checks for CGAL"""

def _check(context):
    cgal = context.env['cgal']
    if cgal is False or cgal is 0:
        context.Message('Checking for CGAL ...')
        context.Result("disabled")
        return False

    ret = context.sconf.CheckLibWithHeader(['CGAL'], 'CGAL/basic.h', 'CXX',
                                           'CGAL_assertion(1);', autoadd=False)
    context.Message('Checking for CGAL ...')
    context.Result(ret)
    if ret:
        context.env['CGAL_LIBS'] = ['CGAL']
        context.env.Append(CPPDEFINES=['IMP_USE_CGAL'])
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
    return ret

def configure_check(env):
    env['CGAL_LIBS'] = ['']
    custom_tests = {'CheckCGAL':_check}
    conf = env.Configure(custom_tests=custom_tests)
    if not env.GetOption('clean') and not env.GetOption('help'):
        conf.CheckCGAL()
    conf.Finish()
