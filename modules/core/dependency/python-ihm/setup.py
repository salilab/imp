#!/usr/bin/env python

try:
    from setuptools import setup, Extension
except ImportError:
    from distutils.core import setup, Extension
import sys
import os

VERSION = "2.11"

copy_args = sys.argv[1:]

# Allow building without the C extension
build_ext = True
if '--without-ext' in copy_args:
    build_ext = False
    copy_args.remove('--without-ext')

# Allow building with Python limited API
if '--py_limited_api' in copy_args:
    copy_args.remove('--py_limited_api')
    # We require Python 3.11 or later since we use PyBUF_WRITE which only
    # became part of the stable API in 3.11
    ext_limited_args = {
        'define_macros': [("Py_LIMITED_API", "0x030B0000")],
        'py_limited_api': True}
    setup_limited_args = {
        'options': {"bdist_wheel": {"py_limited_api": "cp311"}}}
else:
    ext_limited_args = {}
    setup_limited_args = {}

if sys.platform == 'win32':
    # Our use of strdup, strerror should be safe - no need for the Windows
    # compiler to warn about it; we want to use the POSIX name for strdup too
    cargs = ['-D_CRT_SECURE_NO_WARNINGS', '-D_CRT_NONSTDC_NO_WARNINGS']
else:
    cargs = []

if build_ext:
    # Use pre-built SWIG wrappers for stable releases so that end users
    # don't need SWIG installed
    wrap = "src/ihm_format_wrap_%s.c" % VERSION
    if not os.path.exists(wrap):
        wrap = "src/ihm_format.i"
    mod = [Extension("ihm._format",
                     sources=["src/ihm_format.c", "src/cmp.c", wrap],
                     include_dirs=['src'],
                     extra_compile_args=cargs,
                     swig_opts=['-keyword', '-nodefaultctor',
                                '-nodefaultdtor', '-noproxy'],
                     optional=True,
                     **ext_limited_args)]
else:
    mod = []

with open("README.md", "r") as fh:
    long_description = fh.read()

setup(name='ihm',
      version=VERSION,
      script_args=copy_args,
      description='Package for handling IHM mmCIF and BinaryCIF files',
      long_description=long_description,
      long_description_content_type="text/markdown",
      author='Ben Webb',
      author_email='benmwebb@gmail.com',
      url='https://github.com/ihmwg/python-ihm',
      ext_modules=mod,
      packages=['ihm', 'ihm.util'],
      install_requires=['msgpack'],
      license='MIT',
      classifiers=[
          "Programming Language :: Python :: 3",
          "Operating System :: OS Independent",
          "Intended Audience :: Science/Research",
          "Topic :: Scientific/Engineering",
      ],
      **setup_limited_args)
