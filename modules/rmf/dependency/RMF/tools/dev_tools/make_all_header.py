#!/usr/bin/env python

"""Make a header that lists other headers.
usage: make_all_header.py header_name.h include_prefix headers
"""

import sys
import glob
import os

sys.path.append(os.path.split(sys.argv[0])[0])
import python_tools

# Treat an open file as UTF8-encoded, regardless of the locale
if sys.version_info[0] >= 3:
    def open_utf8(fname):
        return open(fname, encoding='UTF8')
else:
    open_utf8 = open


def _add_includes(headers, output):
    for g in headers:
        name = os.path.split(g)[1]
        output.append("#include <%s/" % sys.argv[2] + name + ">")


includepath = sys.argv[1][sys.argv[1].find("include") + len("include") + 1:]

output = ["""/**
 *  \\file %s
 *  \\brief Include all non-deprecated headers in %s.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 */
""" % (includepath, includepath[:-2].replace('/', '.'))]
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
            if 'DEPRECATED_HEADER' in open_utf8(h).read():
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
