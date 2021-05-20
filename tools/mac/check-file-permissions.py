#!/usr/bin/env python

"""Check one or more directories for world- or group-writeable files and
   dangling symlinks.
"""

import os
import stat
import sys


def check_dir(dir, group, world, dangle):
    """Check a single directory for file permissions problems"""
    for dirpath, dirnames, filenames in os.walk(dir):
        for f in dirnames + filenames:
            path = os.path.join(dirpath, f)
            try:
                mode = os.stat(path).st_mode
            except OSError:
                dangle.append(path)
            else:
                if mode & stat.S_IWOTH:
                    world.append(path)
                if mode & stat.S_IWGRP:
                    group.append(path)


def add_errors(errs, files, desc):
    num = len(files)
    if num > 0:
        msg = "%d %s" % (num, desc)
        if num > 1:
            msg += "s"
        errs.append(msg + ": " + ", ".join([repr(x) for x in files]))


def main():
    group = []
    world = []
    dangle = []
    for dir in sys.argv[1:]:
        check_dir(dir, group, world, dangle)
    errs = []
    add_errors(errs, world, "world writeable file")
    add_errors(errs, group, "group writeable file")
    add_errors(errs, dangle, "dangling symlink")
    if len(errs) > 0:
        raise ValueError("; ".join(errs))


if __name__ == '__main__':
    main()
