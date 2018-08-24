#!/usr/bin/env python

from distutils.core import setup, Extension
import sys

copy_args = sys.argv[1:]

# Allow building without the C extension
build_ext = True
if '--without-ext' in copy_args:
    build_ext = False
    copy_args.remove('--without-ext')

if sys.platform == 'win32':
    # Our use of strdup, strerror should be safe - no need for the Windows
    # compiler to warn about it; we want to use the POSIX name for strdup too
    cargs = ['-D_CRT_SECURE_NO_WARNINGS', '-D_CRT_NONSTDC_NO_WARNINGS']
else:
    cargs = []

if build_ext:
    mod = [Extension("ihm._format",
                     sources=["src/ihm_format.c", "src/ihm_format.i"],
                     extra_compile_args=cargs,
                     swig_opts=['-keyword', '-nodefaultctor',
                                '-nodefaultdtor', '-noproxy'])]
else:
    mod = []

setup(name='ihm',
      script_args=copy_args,
      description='Package for handling IHM mmCIF files',
      author='Ben Webb',
      author_email='ben@salilab.org',
      url='https://github.com/ihmwg/python-ihm',
      ext_modules=mod,
      packages=['ihm'])
