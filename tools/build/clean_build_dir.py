#!/usr/bin/env python

"""
Check for left over files that might break build:
- a src/module_name/config.cpp
"""

import os
import sys
import os.path
import shutil
import platform
import tools
import glob
import itertools


def check_config():
    for f in glob.glob(os.path.join("src", "*", "config.cpp")):
        os.unlink(f)


def declare_container():
    for f in glob.glob(os.path.join("include", "IMP", "kernel", "declare_*Container.h")):
        os.unlink(f)


def build_info():
    for f in glob.glob(os.path.join("data", "build_info", "*")):
        os.unlink(f)


def clean_headers():
    for f in [x for l in ["refiner_macros.h",
                          "log.h",
                          "exception.h",
                          "Object.h",
                          "file.h",
                          "sampler_macros.h",
                          "Pointer.h",
                          "WeakPointer.h",
                          "RefCounted.h",
                          "scoring_function_macros.h",
                          "VersionInfo.h",
                          "key_macros.h"] for x in glob.glob(
            os.path.join("include", "IMP", l))]:
        os.unlink(f)


def main():
    check_config()
    declare_container()
    build_info()
    clean_headers()

if __name__ == '__main__':
    main()
