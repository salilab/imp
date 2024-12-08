#!/usr/bin/env python3

"""Make a header that lists other headers.
usage: make_all_header.py header_name.h include_prefix headers
"""

import sys
import glob
import datetime
import os

sys.path.append(os.path.split(sys.argv[0])[0])
import python_tools


def _add_includes(headers, output):
    for g in headers:
        name = os.path.split(g)[1]
        output.append("#include <%s/" % sys.argv[2] + name + ">")


includepath = sys.argv[1][sys.argv[1].find("include") + len("include") + 1:]

year = datetime.datetime.now().year
output = ["""/**
 *  \\file %s
 *  \\brief Include all non-deprecated headers in %s.
 *
 *  Copyright 2007-%d IMP Inventors. All rights reserved.
 */
""" % (includepath, includepath[:-2].replace('/', '.'), year)]
guard = includepath.replace(
    "/",
    "_").replace("\\",
                 "_").replace(".",
                              "_").upper()
output.append("#ifndef %s" % guard)
output.append("#define %s" % guard)

for h in sys.argv[3:]:
    if not h.endswith(".h"):
        pat = os.path.join(h, "*.h")
        orig_h = sorted(glob.glob(pat))
        allh = []
        deprecated_allh = []
        for h in orig_h:
            if 'DEPRECATED_HEADER' in open(h, encoding='UTF8').read():
                deprecated_allh.append(h)
            else:
                allh.append(h)
    else:
        deprecated_allh = []
        allh = [h]
    _add_includes(allh, output)

    if deprecated_allh:
        # SWIG needs all headers (for now)
        output.append("#ifdef IMP_SWIG_WRAPPER")
        _add_includes(deprecated_allh, output)
        output.append("#endif")

output.append("#endif /* %s */" % guard)
python_tools.rewrite(sys.argv[1], "\n".join(output) + "\n")
