"""Simple configure checks for boost"""

from SCons.Script import Exit
import gcc
import os
import scons_tools.utility

def _check(context):
    context.Message( 'Checking for pkg-config... ' )
    #os.environ['PATH']=context.env['ENV']['PATH']
    ret = context.TryAction('pkg-config --atleast-pkgconfig-version=1')[0]
    context.Result( ret==0 )
    return ret==0


def configure_check(env):
    if env.get('pkgconfig') == "auto":
        # We currently only parse pkg-config output for gcc, so don't use
        # pkg-config on non-gcc systems unless the user forces us to
        if not scons_tools.dependency.gcc.get_is_gcc(env):
            env['IMP_HAS_PKG_CONFIG']=False
        else:
            custom_tests = {'CheckPK':_check}
            conf = env.Configure(custom_tests=custom_tests)
            if not conf.CheckPK():
                env['IMP_HAS_PKG_CONFIG']=False
            else:
                env['IMP_HAS_PKG_CONFIG']=True
            conf.Finish()
    elif env.get('pkgconfig')=="no":
        env['IMP_HAS_PKG_CONFIG']=False
    else:
        env['IMP_HAS_PKG_CONFIG']=True


def get_config(context, lcname):
    if not scons_tools.dependency.gcc.get_is_gcc(context.env):
        scons_tools.utility.report_error(context.env,
                                         "pkg-config only supported with g++")
    #print context.env.Execute('pkg-config --cflags-only-I \'%s\'' % lcname)
    retI = os.popen('pkg-config --cflags-only-I \'%s\'' % lcname).read()
    retL = os.popen('pkg-config --libs-only-L \'%s\'' % lcname).read()
    retl = os.popen('pkg-config --libs-only-l \'%s\'' % lcname).read()
    #print retI[:-1]
    #print retL[:-1]
    #print retl[:-1]
    if retI.startswith("-I"):
        inc=retI[2:-1].strip()
    else:
        inc=None
    if retL.startswith("-L"):
        lp=retL[2:-1].strip()
    else:
        lp=None
    libs=[x[2:].strip() for x in retl[:-1].split(" ") if x != '']
    ret= (inc, lp, libs)
    #print ret
    return ret
