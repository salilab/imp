"""Simple configure checks for boost"""

from SCons.Script import Exit
import gcc
import scons_tools.utility

def _check(context):
    context.Message('Checking for CGAL libraries needed... ')
    ret = context.TryRun("""#include <CGAL/config.h>
#include <iostream>

int main(int, char *[]) {
#ifdef CGAL_USE_GMP
  std::cout << "gmp\\n";
#endif
#ifdef CGAL_USE_MPFR
  std::cout << "mpfr\\n";
#endif
#ifdef CGAL_USE_GMPXX
  std::cout << "gmpxx\\n";
#endif
#ifdef CGAL_USE_CORE
  std::cout << "core\\n";
#endif
#ifdef CGAL_HAS_THREADS
  std::cout << "threads\\n";
#endif
  return 0;
}
        """, '.cpp')
    if ret[0]:
        lst= ret[1].split('\n')[:-1] # remove trailing empyt line
        context.Result(" ".join(lst))
        context.env['CGAL_LIB_LIST']=lst
    else:
        context.Result("not found")
        context.env['CGAL_LIB_LIST']=[]
    return ret[0]

def get_libs(env, boost_threads):
    if env.get('cgallibs', None):
        return env.get('cgallibs')
    else:
        custom_tests = {'CheckCGALLibs':_check}
        conf = env.Configure(custom_tests=custom_tests)
        conf.CheckCGALLibs()
        conf.Finish()
        lst= env['CGAL_LIB_LIST']
        if "threads" in lst:
            lst.remove("threads")
            lst.append(boost_threads)
            lst.append('pthread')
        if "core" in lst:
            lst.remove('core')
            #lst[lst.index("core")]="CGAL_Core"
        ret= lst+['m']
        return ret

#["CGAL",'gmp', 'mpfr', 'm', boost_thread, 'pthread']
