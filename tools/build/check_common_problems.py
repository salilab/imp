#!/usr/bin/env python

"""
Check for problems that might break the build in non-IMP repository code:
- a module without a README.md
- missing or incomplete submodules
"""

import os
import sys
import os.path
import shutil
import platform
import tools


def check_readme():
    for module, g in tools.get_modules("."):
        if not os.path.exists(os.path.join(g, "README.md")):
            sys.stderr.write("Each module must have a file 'README.md' describing what what it does and who wrote it.\n")
            sys.stderr.write(module, "does not.\n")
            exit(1)

def check_submodules():
    path = os.path.join(
        "modules",
        "rmf",
        "dependency",
        "RMF_source",
        "README.md")
    if not os.path.exists(path):
        sys.stderr.write("Could not find RMF source in", path,"\n")
        sys.stderr.write("You need to run ./setup_git.py first.\n")
        exit(1)


def main():
    check_readme()
    check_submodules()

if __name__ == '__main__':
    main()
