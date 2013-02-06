from SCons.Script import Exit
import platform

def CheckGCCVisibility(context):
    """Check if the compiler supports setting visibility of symbols"""
    context.Message('Checking whether compiler supports -fvisibility...')
    lastCXXFLAGS = context.env['CXXFLAGS']
    context.env.Append(CXXFLAGS="-fvisibility=hidden")
    text = """
__attribute__ ((visibility("default")))
int main(void)
{ return 0; }
"""
    res = context.TryLink(text, '.cpp')
    context.env.Replace(CXXFLAGS=lastCXXFLAGS)
    if not res:
        context.Result("no")
    else:
        context.env.Append(VIS_CPPDEFINES=['GCC_VISIBILITY'],
                           VIS_CXXFLAGS="-fvisibility=hidden")
        context.Result("yes")
    return res

def configure_check_visibility(env):
    if not(platform.system() != 'win32' and not env['wine']):
        return
    custom_tests = {'CheckGCCVisibility':CheckGCCVisibility}
    conf = env.Configure(custom_tests=custom_tests)
    conf.CheckGCCVisibility()
    conf.Finish()


def CheckGNUHash(context):
    """Disable GNU_HASH-style linking (if found) for backwards compatibility"""
    context.Message('Checking whether GNU_HASH linking should be disabled...')
    lastLINKFLAGS = context.env['LINKFLAGS']
    context.env.Append(LINKFLAGS="-Wl,--hash-style=sysv")
    text = """
int main(void)
{ return 0; }
"""
    res = context.TryLink(text, '.c')
    if not res:
        context.Result("no")
        context.env.Replace(LINKFLAGS=lastLINKFLAGS)
    else:
        context.Result("yes")
    return res

def configure_check_hash(env):
    if env['IMP_USE_PLATFORM_FLAGS'] and (platform.system() == 'Linux'
                                          and env['linksysv']):
        custom_tests = {'CheckGNUHash':CheckGNUHash}
        conf = env.Configure(custom_tests=custom_tests)
        conf.CheckGNUHash()
        conf.Finish()


def get_version(env):
    vstr= env['CXXVERSION']
    svstr= vstr.split(".")
    return float(svstr[0])+.1*float(svstr[1])

def get_is_gcc(env):
    if env.get('CXX', None):
        cxx= env['CXX'];
        return ((cxx.find('g++') != -1 and cxx.find('clang++') == -1)\
                or cxx.find("icc")!= -1\
                or cxx.find("mpicxx") != -1\
                or cxx.find("mpic++") != -1)
    else:
        return False
