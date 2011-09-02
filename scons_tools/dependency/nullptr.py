"""Simple check that compilation works"""
import scons_tools.environment
from SCons.Script import Exit

def _check(context):
    context.Message('Checking if the compiler support nullptr... ')
    ret = context.TryCompile("""

        int main()
        {
            void *v= nullptr;
            return 0;
        }
""", '.cpp')
    context.Result(ret)
    return ret

def configure_check(env):
    custom_tests = {'CheckCompiler':_check}
    tenv= scons_tools.environment.get_test_environment(env)
    conf = tenv.Configure(custom_tests=custom_tests)
    if conf.CheckCompiler():
        env["IMP_COMPILER_NULLPTR"]=True
    else:
        env["IMP_COMPILER_NULLPTR"]=False
    conf.Finish()
