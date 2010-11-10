"""Simple check that compilation works"""

from SCons.Script import Exit

def _check(context):
    context.Message('Checking that the compiler works... ')
    ret = context.TryCompile("""#include <iostream>

        int main()
        {
            std::cout << "Hello world" <<std::endl;
            return 0;
        }
        """, '.cpp')
    context.Result(ret)
    return ret

def configure_check(env):
    custom_tests = {'CheckCompiler':_check}
    conf = env.Configure(custom_tests=custom_tests)
    if conf.CheckCompiler():
        env["COMPILER_OK"]=True
    else:
        env["COMPILER_OK"]=False
    conf.Finish()
