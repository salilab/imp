#!/usr/bin/env python

import coverage
import os
import sys
import subprocess
import tempfile
import shutil

class _TempDir(object):
    "A temporary directory that is automatically removed when no longer needed"
    def __init__(self):
        self.tmpdir = tempfile.mkdtemp()

    def __del__(self):
        shutil.rmtree(self.tmpdir, ignore_errors=True)

    def _get_lcov_path(self):
        """Find the lcov executable in the system PATH"""
        for path in os.environ['PATH'].split(os.pathsep):
            if os.path.isfile(os.path.join(path, 'lcov')):
                return path
        raise OSError("Cannot find lcov executable in PATH")

    def make_lcov_binary(self):
        """Make a patched version of lcov in the temporary directory.
           Running gcov with the --all-blocks (-a) flag causes it
           to get stuck in an endless loop on some files (see GCC Bugzilla
           http://gcc.gnu.org/bugzilla/show_bug.cgi?id=48361) so
           fool geninfo into thinking gcov doesn't support this option."""
        path = self._get_lcov_path()
        shutil.copy(os.path.join(path, 'lcov'), self.tmpdir)
        fin = open(os.path.join(path, 'geninfo'))
        fout = open(os.path.join(self.tmpdir, 'geninfo'), 'w')
        for line in fin:
            line = line.replace('all-blocks', 'DISABLEDall-blocks')
            fout.write(line)
        fin.close()
        fout.close()
        os.chmod(os.path.join(self.tmpdir, 'geninfo'), 0755)
        return os.path.join(self.tmpdir, 'lcov')


def gather_python():
    """Combine all of the .coverage* files into a single .coverage"""
    def _our_abs_file(self, filename):
        return os.path.normcase(os.path.abspath(filename))
    coverage.files.FileLocator.abs_file = _our_abs_file

    cov = coverage.coverage(data_file='coverage/.coverage')
    cov.combine()
    cov.save()

def gather_cpp():
    """Make a single lcov info file from all of the .gcda files"""
    t = _TempDir()
    lcov = t.make_lcov_binary()
    cmd = [lcov, '-c', '-d', '.', '-b', os.getcwd(), '-o', 'coverage/all.info']
    print " ".join(cmd)
    subprocess.check_call(cmd)

def main():
    gather_python()
    gather_cpp()

if __name__ == '__main__':
    main()
