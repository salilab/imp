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

def main():
    check_readme()

if __name__ == '__main__':
    main()
