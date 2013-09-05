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

def main():
    check_config()

if __name__ == '__main__':
    main()
