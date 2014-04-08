#!/usr/bin/python

"""Add a new module to IMP.
"""

import os
import os.path
import sys
import getopt
import shutil
import re


def fix_string(input, modname):
    return input.replace("scratch", modname)\
        .replace("SCRATCH", modname.upper())


def copy_dir(source, dest, modname):
    for x in os.listdir(source):
        if x == ".svn":
            continue
        if x.endswith(".pyc"):
            continue
        if x.endswith(".old"):
            continue
        xspath = os.path.join(source, x)
        print "handling " + xspath,
        if os.path.isdir(xspath):
            xdpath = os.path.join(dest, x)
            print "->" + xdpath
            os.mkdir(xdpath)
            copy_dir(xspath, xdpath, modname)
        else:
            xdpath = os.path.join(dest, fix_string(x, modname))
            print "->" + xdpath
            input = file(xspath, 'r').read()
            if xspath.endswith(".cpp") or xspath.endswith(".h") \
                    or xspath.endswith(".i-in") or xspath.endswith(".py"):
                output = fix_string(input, modname)
            else:
                output = input
            file(xdpath, 'w').write(output)


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
    if os.path.isdir(modpath):
        print "Module already exists"
        return
    print "Creating a new module " + modname
    os.mkdir(modpath)
    copy_dir(os.path.join(impdir, "modules", "scratch"), modpath, modname)


if __name__ == '__main__':
    main()
