#!/usr/bin/env python

"""
Set up a module by
- checking that all required modules and dependencies are found
- creating the config header and .cpp and version check .py
- linking .py files from the bin and benchmarks directories into the build dir

If the module cannot be configured, the script exits with an error.
"""

import sys
from optparse import OptionParser
import os.path
import tools
import glob

parser = OptionParser()
parser.add_option("-n", "--name",
                  dest="name", help="The name of the module.")
parser.add_option("-s", "--source",
                  dest="source", help="The root for IMP source.")
parser.add_option("-v", "--version",
                  dest="version", help="The version file to use.")
parser.add_option("-d", "--datapath",
                  dest="datapath", default="", help="An extra IMP datapath.")

def get_version(options):
    try:
        return open(options.version, "r").read().split("\n")[0]
    except:
        return "develop"


def make_cpp(options):
    dir= os.path.join("src")
    file=os.path.join(dir, "%s_config.cpp"%options.name)
    cpp_template = open(os.path.join(options.source, "tools", "build", "config_templates", "src.cpp"), "r").read()
    try:
        os.makedirs(dir)
    except:
        # exists
        pass
    data={}
    data["filename"]="IMP/%s/%s_config.h"%(options.name, options.name)
    data["cppprefix"]="IMP%s"%options.name.upper().replace("_", "")
    data["name"]= options.name
    data["version"]= get_version(options)
    tools.rewrite(file, cpp_template%data)

def make_version_check(options):
    dir= os.path.join("lib", "IMP", options.name)
    tools.mkdir(dir, clean=False)
    outf= os.path.join(dir, "_version_check.py")
    template="""def check_version(myversion):
  def _check_one(name, expected, found):
    if expected != found:
      message = "Expected version " + expected + " but got " + found + " when loading module " + name + ". Please make sure IMP is properly built and installed and that matching python and C++ libraries are used."
      raise RuntimeError(message)
  version = '%s'
  _check_one('%s', version, myversion)
  """
    tools.rewrite(outf, template%(get_version(options), options.name))


def main():
    (options, args) = parser.parse_args()
    make_cpp(options)
    make_version_check(options)
    sys.exit(0)

if __name__ == '__main__':
    main()
