# WARNING, do not call this file modeller.py as that will confuse the IMP.modeller __init__.py

import os
import os.path
import re
import scons_tools.data
from scons_tools.mypopen import MyPopen
from SCons.Script import File, Alias, Dir, Glob


def _check_default(context):
    """Check for MODELLER in the default Python path"""
    try:
        import modeller
    except ImportError, e:
        context.Result("not found via Python import (specify installation "
                       "path with 'modeller' scons option): %s" % str(e))
        return False
    try:
        ver = modeller.info.version
    except AttributeError, e:
        context.Result("'import modeller' succeeded, but the package does "
                       "not appear to be MODELLER; perhaps a 'modeller.py' "
                       "file in the current directory?")
        return False
    context.Result("found via Python 'import modeller'")
    # No need for the modpy.sh script in this case
    context.env['MODELLER_MODPY'] = ''
    return True

def _check(context):
    """Find Modeller include and library directories"""
    context.Message('Checking for MODELLER...')
    modeller = context.env['modeller']
    if modeller is False or modeller == 0:
        context.Result("disabled (add modeller='yes' to enable)")
        return False
    if modeller is True:
        return _check_default(context)

    # Find MODELLER script
    moddir = "%s/bin" % modeller
    try:
        files = os.listdir(moddir)
    except OSError, e:
        context.Result("could not find MODELLER directory %s: %s" % (moddir, e))
        return False
    files.sort()
    r = re.compile('mod(SVN|\d+[v.]\d+)$')
    files = [f for f in files if r.match(f)]
    if len(files) == 0:
        context.Result("could not find MODELLER script in %s" % moddir)
        return False
    # Last matching entry is probably the latest version:
    modbin = os.path.join(moddir, files[-1])
    try:
        p = MyPopen(modbin + " -")
        print >> p.stdin, "print 'EXE type: ', info.exe_type"
        p.stdin.close()
    except IOError, e:
        context.Result("could not run MODELLER script %s: %s" % (modbin, e))
        return False
    err = p.stderr.read()
    exetype = None
    for line in p.stdout:
        if line.startswith("EXE type"):
            exetype=line[11:].rstrip('\r\n')
    ret = p.wait()
    if exetype is None:
        if err or ret != 0:
            context.Result("could not run MODELLER script %s: %d, %s" \
                           % (modbin, ret, err))
        else:
            context.Result("unknown error running MODELLER script %s" % modbin)
        return False
    platform = context.env['PLATFORM']
    if exetype == 'i386-w32':
        if platform != 'win32':
            context.Result("MODELLER is built for Windows, but this is not " + \
                           "a Windows scons run (tip: can run on Linux " + \
                           "using Wine with 'scons wine=true'")
            return False
    else:
        if platform == 'win32':
            context.Result("this is a Windows scons run, but this is not a " + \
                           "Windows MODELLER binary")
            return False
    modpy = "%s/bin/modpy.sh" % modeller
    # If the modpy.sh script doesn't exist, assume that Modeller will work
    # without it (e.g. on Macs, using the binary .dmg install):
    if not os.path.exists(modpy):
        modpy = ''
    context.env['MODELLER_MODPY'] = modpy
    context.Result(modeller)
    return True


def configure_check(env):
    custom_tests = {'CheckModeller':_check}
    conf = env.Configure(custom_tests=custom_tests)
    #if not env.GetOption('clean') and not env.GetOption('help'):
    hm = conf.CheckModeller()
    config=["ok=%s"%hm]
    open(File("#/build/data/build_info/modeller").abspath, "w").write("\n".join(config))
    conf.Finish()
