#!/usr/bin/env python

import os
import sys
import os.path
import shutil
import platform
import tools
from optparse import OptionParser

# main loops


def generate_all_cpp(source):
    target = os.path.join("src")
    tools.mkdir(target)
    for module, g in tools.get_modules(source):
        sources = tools.get_glob([os.path.join(g, "src", "*.cpp")])\
            + tools.get_glob([os.path.join(g, "src", "internal", "*.cpp")])
        targetf = os.path.join(target, module + "_all.cpp")
        sources.sort()
        tools.rewrite(
            targetf, "\n".join(["#include <%s>" %
                                os.path.abspath(s) for s in sources]) + '\n')

parser = OptionParser()
parser.add_option("-s", "--source", dest="source",
                  help="IMP source directory.")


def main():
    (options, args) = parser.parse_args()
    generate_all_cpp(options.source)

if __name__ == '__main__':
    main()
