"""Simple configure checks for CGAL"""
import checks

def _check(context):
    cgal = context.env['cgal']
    if cgal is False or cgal is 0:
        context.Message('Checking for CGAL ...')
        context.Result("disabled")
        return False

    ret= checks.check_lib(context, lib='ANN', header=['ANN/ANN.h'],
                          body='',
                          extra_libs=[])
    context.Message('Checking for ANN ...')
    context.Result(ret[0])

    return ret[0]

def configure_check(env):
    env['ANN_LIBS'] = ['']
    custom_tests = {'CheckANN':_check}
    conf = env.Configure(custom_tests=custom_tests)
    #if not env.GetOption('clean') and not env.GetOption('help'):
    if conf.CheckANN():
        env.Append(IMP_BUILD_SUMMARY=["ANN support enabled."])
    else:
        env.Append(IMP_BUILD_SUMMARY=["ANN support disabled."])
    conf.Finish()
