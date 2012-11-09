"""Simple check that compilation works"""
import scons_tools.environment
from SCons.Script import Exit

def _check(context):
    context.Message('Checking if the compiler supports final... ')
    ret = context.TryCompile("""
struct Base1 final { };
        int main()
        {
            return 0;
        }
""", '.cpp')
    context.Result(ret)
    return ret

def configure_check(env):
    if env.get("c++11_final", "auto") == "auto":
        custom_tests = {'CheckCompiler':_check}
        tenv= scons_tools.environment.get_test_environment(env)
        conf = tenv.Configure(custom_tests=custom_tests)
        if conf.CheckCompiler():
            env["IMP_COMPILER_FINAL"]=True
        else:
            env["IMP_COMPILER_FINAL"]=False
        conf.Finish()
    elif env.get("c++11_final", "auto")=="no":
        print "Reading if the compiler supports final...no"
        env["IMP_COMPILER_FINAL"]=False
    else:
        print "Reading if the compiler supports final...yes"
        env["IMP_COMPILER_FINAL"]=True
