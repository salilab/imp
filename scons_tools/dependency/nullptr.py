"""Simple check that compilation works"""
import scons_tools.environment
from SCons.Script import Exit

def _check(context):
    context.Message('Checking if the compiler supports nullptr... ')
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
    if env.get("nullptr", "auto") == "auto":
        custom_tests = {'CheckCompiler':_check}
        tenv= scons_tools.environment.get_test_environment(env)
        conf = tenv.Configure(custom_tests=custom_tests)
        if conf.CheckCompiler():
            env["IMP_COMPILER_NULLPTR"]=True
        else:
            env["IMP_COMPILER_NULLPTR"]=False
        conf.Finish()
    elif env.get("nullptr", "auto")=="no":
        print "Reading if the compiler supports nullptr...no"
        env["IMP_COMPILER_NULLPTR"]=False
    else:
        print "Reading if the compiler supports nullptr...yes"
        env["IMP_COMPILER_NULLPTR"]=True
