#!/usr/bin/python
import _tools
from optparse import OptionParser
import glob
import os.path

header_template ="""#include <IMP/%(module)s/%(file)s>

%(namespacebegin)s
using namespace IMP::%(module)s;
%(namespaceend)s
"""

parser = OptionParser()
parser.add_option("-m", "--module", dest="module", default="",
                  help="Name of the source module.")
parser.add_option("-a", "--alias",
                  dest="alias", help="The name of the module alias.")


def main():
    (options, args) = parser.parse_args()
    _tools.mkdir("include/IMP/%s"%options.alias)
    for h in glob.glob("include/IMP/%s/*.h"%options.module):
        filename= os.path.split(h)[1]
        header= header_template%{"module": options.module,
                                 "file":filename,
                                 "namespacebegin": "namespace IMP { namespace %s {"%options.alias,
                                 "namespaceend": "} }"}
        open("include/IMP/%s/%s"%(options.alias, filename), "w").write(header)

if __name__ == '__main__':
    main()
