"""Simple check that compilation works"""
import scons_tools.environment
from SCons.Script import Exit

def _check(context):
    context.Message('Checking if the compiler supports _Pragma... ')
    ret = context.TryCompile("""
#define MY_MACRO \
_Pragma ("warn pointless warning")

MY_MACRO

        int main()
        {
            return 0;
        }
""", '.cpp')
    context.Result(ret)
    return ret

def configure_check(env):
    if env.get("c++11_pragma", "auto") == "auto":
        custom_tests = {'CheckCompiler':_check}
        tenv= scons_tools.environment.get_test_environment(env)
        conf = tenv.Configure(custom_tests=custom_tests)
        if conf.CheckCompiler():
            env["IMP_COMPILER_PRAGMA"]=True
        else:
            env["IMP_COMPILER_PRAGMA"]=False
        conf.Finish()
    elif env.get("c++11_pragma", "auto")=="no":
        print "Reading if the compiler supports _Pragma...no"
        env["IMP_COMPILER_PRAGMA"]=False
    else:
        print "Reading if the compiler supports _Pragam...yes"
        env["IMP_COMPILER_PRAGMA"]=True
