#!/usr/bin/env python

import os
import shutil


def clean_gcdas():
    for dirpath, dirnames, filenames in os.walk('.'):
        for f in filenames:
            if f.endswith('.gcda'):
                os.unlink(os.path.join(dirpath, f))


def setup_python_coverage():
    if os.path.exists('coverage'):
        shutil.rmtree('coverage')
    os.mkdir('coverage')
    open('coverage/sitecustomize.py', 'w').write("""
import coverage
import atexit
import os

_cov = coverage.coverage(branch=True, data_suffix=True, auto_data=True,
                         data_file='%s')
_cov.start()

def _coverage_cleanup(c):
    c.stop()
atexit.register(_coverage_cleanup, _cov)
""" % os.path.abspath('coverage/.coverage'))

def main():
    clean_gcdas()
    setup_python_coverage()

if __name__ == '__main__':
    main()
