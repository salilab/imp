"""Simple check that compilation works"""
import scons_tools.environment
from SCons.Script import Exit

def _check(context):
    context.Message('Checking if the compiler supports C++11 enums... ')
    ret = context.TryCompile("""
enum class Enumeration {
    Val1,
    Val2,
    Val3 = 100,
    Val4 // = 101
};

        int main()
        {
            return 0;
        }
""", '.cpp')
    context.Result(ret)
    return ret

def configure_check(env):
    if env.get("c++11_enums", "auto") == "auto":
        custom_tests = {'CheckCompiler':_check}
        tenv= scons_tools.environment.get_test_environment(env)
        conf = tenv.Configure(custom_tests=custom_tests)
        if conf.CheckCompiler():
            env["IMP_COMPILER_ENUMS"]=True
        else:
            env["IMP_COMPILER_ENUMS"]=False
        conf.Finish()
    elif env.get("c++11_enums", "auto")=="no":
        print "Reading if the compiler supports C++11 enums...no"
        env["IMP_COMPILER_ENUMS"]=False
    else:
        print "Reading if the compiler supports C++11 enums...yes"
        env["IMP_COMPILER_ENUMS"]=True
