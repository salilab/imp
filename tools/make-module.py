#!/usr/bin/env python

"""Add a new module to IMP.
"""

from __future__ import print_function

import os
import os.path
import sys
import getopt
import shutil
import re
import time

def fix_string(input, modname):
    return input.replace("scratch", modname)\
        .replace("SCRATCH", modname.upper())


def copy_dir(source, dest, modname):
    for x in os.listdir(source):
        if x == ".svn" or x == 'CMakeLists.txt':
            continue
        if x.endswith(".pyc"):
            continue
        if x.endswith(".old"):
            continue
        xspath = os.path.join(source, x)
        #print("handling " + xspath, end='')
        if os.path.isdir(xspath):
            xdpath = os.path.join(dest, x)
            #print("->" + xdpath)
            os.mkdir(xdpath)
            copy_dir(xspath, xdpath, modname)
        else:
            xdpath = os.path.join(dest, fix_string(x, modname))
            #print("->" + xdpath)
            input = file(xspath, 'r').read()
            if xspath.endswith(".cpp") or xspath.endswith(".h") \
                    or xspath.endswith(".i-in") or xspath.endswith(".py") \
                    or xspath.endswith(".md"):
                output = fix_string(input, modname)
            else:
                output = input
            file(xdpath, 'w').write(output)

def make_readme(modpath):
    """Overwrite README.md from scratch module with a new one"""
    outf = os.path.join(modpath, 'README.md')
    with open(outf, 'w') as fh:
        fh.write("""\\brief New module created by make-module.py on %s

Fill in this file with a description of your module.

# Info

_Author(s)_: (full names of author(s))

_Maintainer_: (GitHub name of active maintainer)

_License_: None

_Publications_:
- None
""" % time.strftime("%c"))

def main():
    impdir = os.path.split(os.path.split(sys.argv[0])[0])[0]
    if len(sys.argv) < 2:
        print("Usage: %s module_name" % sys.argv[0])
        return
    modname = sys.argv[1]
    if len(sys.argv) == 3:
        modpath = sys.argv[2]
    else:
        modpath = os.path.join("modules", modname)
    if not re.match('[a-zA-Z0-9_]+$', modname):
        print("Module names can only contain letters, numbers, and underscores")
        return
    if os.path.isdir(modpath):
        print("Module already exists")
        return
    print("Creating a new module " + modname + " in " + modpath)
    os.mkdir(modpath)
    copy_dir(os.path.join(impdir, "modules", "scratch"), modpath, modname)
    make_readme(modpath)


if __name__ == '__main__':
    main()
