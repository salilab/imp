#!/usr/bin/env python

"""
Check for problems that might breatk the build in non-IMP repository code:
- a module/application without dependencies.py
- a module/application without a README.md
"""

import os
import sys
import os.path
import shutil
import platform
import tools


def check_dependencies():
    for module, g in tools.get_modules("."):
        if not os.path.exists(os.path.join(g, "dependencies.py")):
            print >> sys.stderr, "Each module must have a file 'dependencies.py' describing what other modules and external dependencies it needs."
            print >> sys.stderr, module, "does not."
            exit(1)
    for app, g in tools.get_applications("."):
        if not os.path.exists(os.path.join(g, "dependencies.py")):
            print >> sys.stderr, "Each application must have a file 'dependencies.py' describing what other modules and external dependencies it needs."
            print >> sys.stderr, app, "does not."
            exit(1)

def check_readme():
    for module, g in tools.get_modules("."):
        if not os.path.exists(os.path.join(g, "README.md")):
            print >> sys.stderr, "Each module must have a file 'README.md' describing what what it does and who wrote it."
            print >> sys.stderr, module, "does not."
            exit(1)
    for app, g in tools.get_applications("."):
        if not os.path.exists(os.path.join(g, "README.md")):
            print >> sys.stderr, "Each application must have a file 'README.md' describing what who wrote it and what it does."
            print >> sys.stderr, app, "does not."
            exit(1)

def main():
    check_dependencies()
    check_readme()

if __name__ == '__main__':
    main()
