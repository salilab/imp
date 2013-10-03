"""@namespace IMP.parallel.subproc Subprocess handling."""

import sys
try:
    import subprocess
except ImportError:
    # Work with Python 2.3, which doesn't ship with subprocess
    from IMP.parallel import compat_subprocess as subprocess

class _Popen4(subprocess.Popen):
    """Utility class to provide a portable way to spawn a child process and
       communicate with its stdin and combined stdout/stderr."""

    def __init__(self, cmd):
        # shell isn't needed on Win32, and may not be found under wine anyway
        shell = (sys.platform != "win32")
        subprocess.Popen.__init__(self, cmd, shell=shell, stdin=subprocess.PIPE,
                                  stdout=subprocess.PIPE,
                                  stderr=subprocess.STDOUT)

    def require_clean_exit(self):
        """Make sure the child exited with a zero return code"""
        r = self.wait()
        if r != 0:
            raise IOError("Process failed with exit status %d" % r)

if sys.platform == 'win32':
    def _run_background(cmdline, out):
        """Run a process in the background and direct its output to a file"""
        print "%s > %s" % (cmdline, out)
        try:
            # shell isn't needed on Win32, and may not be found under wine
            # anyway
            p = subprocess.Popen(cmdline, shell=False, stdout=open(out, 'w'),
                                 stderr=subprocess.STDOUT)
        # Ignore Windows "file not found" errors, so that behavior is consistent
        # between Unix and Windows
        except WindowsError, detail:
            print("WindowsError: %s (ignored)" % detail)

else:

    def _run_background(cmdline, out):
        """Run a process in the background and direct its output to a file"""
        print "%s > %s" % (cmdline, out)
        subprocess.Popen(cmdline, shell=True, stdout=open(out, 'w'),
                         stderr=subprocess.STDOUT)
