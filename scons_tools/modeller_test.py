# WARNING, do not call this file modeller.py as that will confuse the IMP.modeller __init__.py

import os
import os.path
import re

# On older Pythons that don't have subprocess, fall back to os.popen3
try:
    import subprocess
    def MyPopen(cmd):
        return subprocess.Popen(cmd, shell=True, stdin=subprocess.PIPE,
                                stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                close_fds=True)
except ImportError:
    class MyPopen(object):
        def __init__(self, cmd):
            (self.stdin, self.stdout, self.stderr) \
                = os.popen3(cmd, 't', -1)
        def wait(self):
            return 0

def _check(context):
    """Find Modeller include and library directories"""
    context.Message('Checking for MODELLER...')
    modeller = context.env['modeller']
    if modeller is False or modeller is 0:
        context.Result("not found")
        return False
    # Find MODELLER script
    moddir = "%s/bin" % modeller
    try:
        files = os.listdir(moddir)
    except OSError, e:
        context.Result("could not find MODELLER directory %s: %s" % (moddir, e))
        return False
    files.sort()
    r = re.compile('mod(SVN|\d+v\d+)$')
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
    include = ['%s/src/include' % modeller,
               '%s/src/include/%s' % (modeller, exetype)]
    platform = context.env['PLATFORM']
    if exetype == 'i386-w32':
        libpath = ['%s/src/main' % modeller]
        if platform != 'win32':
            context.Result("MODELLER is built for Windows, but this is not " + \
                           "a Windows scons run (tip: can run on Linux " + \
                           "using Wine with 'scons wine=true'")
            return False
    else:
        libpath = ['%s/lib/%s' % (modeller, exetype)]
        if platform == 'win32':
            context.Result("this is a Windows scons run, but this is not a " + \
                           "Windows MODELLER binary")
            return False
    libs = ["modeller", "saxs"]
    if exetype in ('mac10v4-xlf', 'mac10v4-gnu'):
        libs += ["hdf5", "hdf5_hl"]
    elif exetype == 'mac10v4-intel':
        libs += ["hdf5", "hdf5_hl", "imf", "svml", "ifcore", "irc"]
    modpy = "%s/bin/modpy.sh" % modeller
    # If the modpy.sh script doesn't exist, assume that Modeller will work
    # without it (e.g. on Macs, using the binary .dmg install):
    if not os.path.exists(modpy):
        modpy = ''
    context.env['MODELLER_MODPY'] = modpy
    context.env['MODELLER_EXETYPE'] = exetype
    context.env['MODELLER_CPPPATH'] = include
    context.env['MODELLER_LIBPATH'] = libpath
    context.env['MODELLER_LIBS'] = libs
    context.Result(modeller)
    return True


def configure_check(env):
    custom_tests = {'CheckModeller':_check}
    conf = env.Configure(custom_tests=custom_tests)
    if not env.GetOption('clean') and not env.GetOption('help'):
        env['HAS_MODELLER'] = conf.CheckModeller()
    else:
        env['HAS_MODELLER']=False
    conf.Finish()

def fail(env, target, source):
    print """
  ERROR: Modeller is required to build %s.

  Use the modeller command line option (or options file) to set the
  directory where Modeller is installed (run 'scons -h' for help.)
""" % target[0]
    return 1
