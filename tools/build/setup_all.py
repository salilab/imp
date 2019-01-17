#!/usr/bin/env python

import os
import sys
import os.path
import shutil
import platform
import tools
from optparse import OptionParser

# main loops


def generate_all_cpp(modules):
    target = os.path.join("src")
    tools.mkdir(target)
    gen = tools.CPPFileGenerator()
    for module in modules:
        sources = tools.get_glob([os.path.join(module.path, "src", "*.cpp")])\
                  + tools.get_glob([os.path.join(module.path, "src",
                                                 "internal", "*.cpp")])
        targetf = os.path.join(target, module.name + "_all.cpp")
        sources.sort()
        gen.write(targetf, "\n".join(["#include <%s>" %
                                os.path.abspath(s) for s in sources]) + '\n')

parser = OptionParser()
parser.add_option("--build_dir", help="IMP build directory", default=None)
parser.add_option("-s", "--source", dest="source",
                  help="IMP source directory.")


def main():
    (options, args) = parser.parse_args()
    mf = tools.ModulesFinder(source_dir=options.source,
                             external_dir=options.build_dir)
    generate_all_cpp([x for x in mf.values()
                      if isinstance(x, tools.SourceModule)])

if __name__ == '__main__':
    main()
