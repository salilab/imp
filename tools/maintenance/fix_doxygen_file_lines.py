#!/usr/bin/env python

import sys
import re

"""Rewrite the doxygen \\file lines to have the full path to the file."""


def fix(filename):
    contents = open(filename, "r").read()
    contents = re.sub(
        "\\\\file .*\\.h",
        "\\\\file " + filename[len("build/include/"):],
        contents,
        1)
    contents = re.sub(
        "\\\\file .*/.*\\.h",
        "\\\\file " + filename[len("build/include/"):],
        contents,
        1)
    f = open(filename, "wr")
    f.truncate()
    f.write(contents)

if __name__ == '__main__':
    for f in sys.argv[1:]:
        fix(f)
