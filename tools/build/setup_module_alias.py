#!/usr/bin/python
import _tools
from optparse import OptionParser
import glob
import os.path

header_template ="""/** \\file IMP%(slashalias)s/%(file)s
 \\brief Import IMP/%(module)s/%(file)s in the namespace.
*/
#include <IMP/%(module)s/%(file)s>

%(namespacebegin)s
using namespace ::IMP::%(module)s;
%(namespaceend)s
"""

internal_header_template ="""#include <IMP/%(module)s/internal/%(file)s>

%(namespacebegin)s
using namespace ::IMP::%(module)s;
%(namespaceend)s
"""

allh_template = """#include <IMP/%(module)s.h>

%(namespacebegin)s
using namespace ::IMP::%(module)s;
%(namespaceend)s
"""

parser = OptionParser()
parser.add_option("-m", "--module", dest="module", default="",
                  help="Name of the source module.")
parser.add_option("-a", "--alias",
                  dest="alias", help="The name of the module alias.")
parser.add_option("-s", "--source",
                  dest="source", help="IMP source directory.")


def main():
    (options, args) = parser.parse_args()
    _tools.mkdir("include/IMP/%s"%options.alias)
    _tools.mkdir("include/IMP/%s/internal"%options.alias)
    var={"module": options.module}
    if options.alias=="":
        var["namespacebegin"]="namespace IMP {"
        var["namespaceend"]="}"
        var["slashalias"]=""
    else:
        var["namespacebegin"]="namespace IMP { namespace %s {"%options.alias
        var["namespaceend"]="} }"
        var["slashalias"]="/"+options.alias
    for h in glob.glob(os.path.join(options.source, "modules", options.module, "include", "*.h")):
        filename= os.path.split(h)[1]
        var["file"]=filename
        header= header_template%var
        _tools.rewrite("include/IMP%s/%s"%(var["slashalias"], filename), header)
    for h in glob.glob(os.path.join(options.source, "modules", options.module, "include", "internal", "*.h")):
        filename= os.path.split(h)[1]
        var["file"]=filename
        header= internal_header_template%var
        _tools.rewrite("include/IMP/%s/internal/%s"%(options.alias, filename), header)
    allh= allh_template%var
    _tools.rewrite("include/IMP%s.h"%var["slashalias"], allh)

if __name__ == '__main__':
    main()
