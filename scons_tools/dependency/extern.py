"""Simple check that compilation works"""
import scons_tools.environment
from SCons.Script import Exit

def _check(context):
    context.Message('Checking if the compiler supports extern... ')
    ret = context.TryCompile("""
    template <class T>
    class Foo{};
    extern template class Foo<int>;

    int main()
        {
            return 0;
        }
""", '.cpp')
    context.Result(ret)
    return ret

def configure_check(env):
    if env.get("c++11_extern", "auto") == "auto":
        custom_tests = {'CheckCompiler':_check}
        tenv= scons_tools.environment.get_test_environment(env)
        conf = tenv.Configure(custom_tests=custom_tests)
        if conf.CheckCompiler():
            env["IMP_COMPILER_EXTERN"]=True
        else:
            env["IMP_COMPILER_EXTERN"]=False
        conf.Finish()
    elif env.get("c++11_extern", "auto")=="no":
        print "Reading if the compiler supports extern...no"
        env["IMP_COMPILER_EXTERN"]=False
    else:
        print "Reading if the compiler supports extern...yes"
        env["IMP_COMPILER_EXTERN"]=True
