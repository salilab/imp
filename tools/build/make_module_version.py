#!/usr/bin/env python

"""
Set up a module by
- checking that all required modules and dependencies are found
- creating the config header and .cpp and version check .py
- linking .py files from the bin and benchmarks directories into the build dir

If the module cannot be configured, the script exits with an error.
"""

import sys
from argparse import ArgumentParser
import os.path
import tools

TOPDIR = os.path.abspath(os.path.dirname(__file__))

parser = ArgumentParser()
parser.add_argument("-n", "--name",
                    dest="name", help="The name of the module.",
                    required=True)
parser.add_argument("-s", "--source",
                    dest="source", help="The root for IMP source.",
                    required=True)
parser.add_argument("-d", "--datapath",
                    dest="datapath", default="", help="An extra IMP datapath.")


def make_cpp(args):
    dir = os.path.join("src")
    file = os.path.join(dir, "%s_config.cpp" % args.name)
    cpp_template = tools.CPPFileGenerator(
        os.path.join(TOPDIR, "config_templates", "src.cpp"))
    try:
        os.makedirs(dir)
    except OSError:
        # exists
        pass
    data = {}
    if args.name == 'kernel':
        data["filename"] = "IMP/%s_config.h" % args.name
    else:
        data["filename"] = "IMP/%s/%s_config.h" % (args.name, args.name)
    data["cppprefix"] = "IMP%s" % args.name.upper().replace("_", "")
    data["name"] = args.name
    data["version"] = tools.get_module_version(args.name, args.source)
    cpp_template.write(file, data)


def make_version_check(args):
    dir = os.path.join("lib", "IMP",
                       "" if args.name == 'kernel' else args.name)
    tools.mkdir(dir, clean=False)
    version = tools.get_module_version(args.name, args.source)
    outf = os.path.join(dir, "_version_check.py")
    template = """def check_version(myversion):
  def _check_one(name, expected, found):
      if expected != found:
          raise RuntimeError(
              "Expected version " + expected + " but got " + found
              + " when loading module " + name
              + ". Please make sure IMP is properly built and installed "
              + "and that matching python and C++ libraries are used.")
  version = '%s'
  _check_one('%s', version, myversion)
  """
    g = tools.PythonFileGenerator()
    g.write(outf, template % (version, args.name))


def main():
    args = parser.parse_args()
    make_cpp(args)
    make_version_check(args)
    sys.exit(0)


if __name__ == '__main__':
    main()
