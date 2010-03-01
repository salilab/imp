from SCons.Script import Exit
import checks
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
    res = context.TryLink(text, '.c')
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
    if not (platform.system() == 'Linux' and env['linksysv']):
        return
    custom_tests = {'CheckGNUHash':CheckGNUHash}
    conf = env.Configure(custom_tests=custom_tests)
    conf.CheckGNUHash()
    conf.Finish()
