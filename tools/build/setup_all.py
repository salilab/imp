#!/usr/bin/env python

import os
import os.path
import tools
from argparse import ArgumentParser

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
        gen.write(targetf,
                  "\n".join("#include <%s>"
                            % os.path.abspath(s) for s in sources) + '\n')


parser = ArgumentParser()
parser.add_argument("--build_dir", help="IMP build directory", default=None)
parser.add_argument("--module_name", help="Module name", default=None)
parser.add_argument("-s", "--source", dest="source",
                    help="IMP source directory.")


def main():
    args = parser.parse_args()
    mf = tools.ModulesFinder(source_dir=args.source,
                             external_dir=args.build_dir,
                             module_name=args.module_name)
    generate_all_cpp([x for x in mf.values()
                      if isinstance(x, tools.SourceModule)])


if __name__ == '__main__':
    main()
