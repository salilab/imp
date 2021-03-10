#!/usr/bin/env python

"""
Set up the things so doyxgen can be run:
- wrap the README.mds into .dox files
- generate the doxyfiles

No repository directories are changed.
"""

import os
import os.path
import tools
from optparse import OptionParser

# link all the dox files and other documentation related files from the
# source tree into the build tree


def link_dox(source):
    for subdir in ("ref", "manual"):
        target = os.path.join("doxygen", subdir)
        tools.mkdir(target)
    tools.link_dir(os.path.join(source, "doc", "ref"),
                   os.path.join("doc", "ref"),
                   match=["*.png", "*.pdf", "*.gif"], clean=False)
    tools.link_dir(os.path.join(source, "doc", "manual", "images"),
                   os.path.join("doc", "manual"),
                   match=["*.png", "*.pdf", "*.gif"], clean=False)


parser = OptionParser()
parser.add_option("-s", "--source", dest="source",
                  help="IMP source directory.")


def main():
    (options, args) = parser.parse_args()

    link_dox(options.source)


if __name__ == '__main__':
    main()
