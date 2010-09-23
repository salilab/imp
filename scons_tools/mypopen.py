# On older Pythons that don't have subprocess, fall back to os.popen3
try:
    import subprocess
    def MyPopen(cmd):
        return subprocess.Popen(cmd, shell=True, stdin=subprocess.PIPE,
                                stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                close_fds=True)
except ImportError:
    import os
    class MyPopen(object):
        def __init__(self, cmd):
            (self.stdin, self.stdout, self.stderr) \
                = os.popen3(cmd, 't', -1)
        def wait(self):
            return 0
