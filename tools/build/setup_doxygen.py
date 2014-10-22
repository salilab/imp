#!/usr/bin/env python

"""
Set up the things so doyxgen can be run:
- wrap the README.mds into .dox files
- generate the doxyfiles

No repository directories are changed.
"""

import os
import sys
import os.path
import shutil
import platform
import tools
from optparse import OptionParser

# link all the dox files and other documentation related files from the source tree
# into the build tree


def link_dox(source):
    target = os.path.join("doxygen")
    tools.mkdir(target)
    for module, g in tools.get_modules(source):
        tools.link_dir(os.path.join(g, "doc"),
                       os.path.join("doc", "html", module),
                       match=["*.png", "*.pdf"], clean=False)
    for app, g in tools.get_applications(source):
        tools.link_dir(g, os.path.join("doc", "html", app),
                       match=["*.png", "*.pdf"], exclude=["README.md"],
                       clean=False)
    tools.link_dir(
        os.path.join(source, "doc"), os.path.join("doc", "html"), match=["*.png", "*.pdf"],
        clean=False)
    tools.link_dir(os.path.join(source, "doc", "tutorial"),
                   os.path.join("doc", "tutorial"), match=["*.png", "*.pdf"],
                   clean=False)

parser = OptionParser()
parser.add_option("-s", "--source", dest="source",
                  help="IMP source directory.")


def main():
    (options, args) = parser.parse_args()

    link_dox(options.source)

if __name__ == '__main__':
    main()
