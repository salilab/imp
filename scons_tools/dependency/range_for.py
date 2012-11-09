"""Simple check that compilation works"""
import scons_tools.environment
from SCons.Script import Exit

def _check(context):
    context.Message('Checking if the compiler supports range based for loops... ')
    ret = context.TryCompile("""
    #include <vector>
        int main()
        {
            std::vector<int> v;
            for (int i : v ) {
            }
            return 0;
        }
""", '.cpp')
    context.Result(ret)
    return ret

def configure_check(env):
    if env.get("c++11_range_for", "auto") == "auto":
        custom_tests = {'CheckCompiler':_check}
        tenv= scons_tools.environment.get_test_environment(env)
        conf = tenv.Configure(custom_tests=custom_tests)
        if conf.CheckCompiler():
            env["IMP_COMPILER_FOR"]=True
        else:
            env["IMP_COMPILER_FOR"]=False
        conf.Finish()
    elif env.get("c++11_range_for", "auto")=="no":
        print "Reading if the compiler supports range based for loops...no"
        env["IMP_COMPILER_FOR"]=False
    else:
        print "Reading if the compiler supports range based for loops...yes"
        env["IMP_COMPILER_FOR"]=True
