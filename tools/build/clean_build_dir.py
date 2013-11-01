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


def check_config():
    for f in glob.glob(os.path.join("src", "*", "config.cpp")):
        os.unlink(f)

def declare_container():
    for f in glob.glob(os.path.join("include", "IMP", "kernel", "declare_*Container.h")):
        os.unlink(f)

def build_info():
    for f in glob.glob(os.path.join("data", "build_info", "*")):
        os.unlink(f)

def main():
    check_config()
    declare_container()
    build_info()

if __name__ == '__main__':
    main()
