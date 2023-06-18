#!/usr/bin/env python

"""Like cmake -E copy_directory, but dereference any symlinks"""

import shutil
import sys


def main():
    if len(sys.argv) != 3:
        raise ValueError("Usage: %s src dst" % sys.argv[0])
    _, src, dst = sys.argv
    shutil.copytree(src, dst, symlinks=False)


if __name__ == '__main__':
    main()
