"""Simple check that compilation works"""

from SCons.Script import Exit

def _check(context):
    context.Message('Checking that the compiler works... ')
    ret = context.TryRun("""#include <iostream>

        int main()
        {
            std::cout << "Hello world" <<std::endl;
            return 0;
        }
        """, '.cpp')
    context.Result(ret[0])
    return ret[0]

def configure_check(env):
    custom_tests = {'CheckCompiler':_check}
    conf = env.Configure(custom_tests=custom_tests)
    if not env.GetOption('clean') and not env.GetOption('help') \
       and not conf.CheckCompiler():
        Exit("""
No working compiler found. Please make sure that g++ or another
compiler recognized by scons can be found in your path and that all
the passed compiler options (cxxflags, linkflags) are correct.
""")
    conf.Finish()
