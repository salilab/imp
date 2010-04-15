"""Simple configure checks for CGAL"""
import checks

def _check(context):
    ret= checks.check_lib(context, lib='ANN', header=['ANN/ANN.h'],
                          body='',
                          extra_libs=[])
    context.env['ANN_LIBS']=ret[1]
    context.Message('Checking for ANN ...')
    if ret[0]:
        context.Result(" ".join(ret[1]))
    else:
        context.Result(False)
    return ret[0]

def configure_check(env):
    env['ANN_LIBS'] = None
    custom_tests = {'CheckANN':_check}
    conf = env.Configure(custom_tests=custom_tests)
    #if not env.GetOption('clean') and not env.GetOption('help'):
    if conf.CheckANN():
        env.Append(IMP_BUILD_SUMMARY=["ANN support enabled."])
    else:
        env.Append(IMP_BUILD_SUMMARY=["ANN support disabled."])
    conf.Finish()
