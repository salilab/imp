#!/usr/bin/env python3

import coverage
import os
import subprocess
import tempfile
import shutil


def gather_python():
    """Combine all of the .coverage* files into a single .coverage"""

    def _our_abs_file(self, filename):
        return os.path.normcase(os.path.abspath(filename))
    if hasattr(coverage.files, 'FileLocator'):
        coverage.files.FileLocator.abs_file = _our_abs_file

    cov = coverage.coverage(data_file='coverage/.coverage')
    cov.combine()
    cov.save()


def gather_cpp():
    """Make a single lcov info file from all of the .gcda files"""
    cmd = ['/bin/lcov', '-c', '-d', '.', '-b', os.getcwd(),
           '-o', 'coverage/all.info']
    print(" ".join(cmd))
    subprocess.check_call(cmd)


def main():
    gather_python()
    gather_cpp()


if __name__ == '__main__':
    main()
