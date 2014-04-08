#!/usr/bin/env python

"""Fix some broken links in the documentation that result from doxygen bugs:

   - doxygen adds broken links from Python source pages (to namespace*.html)
     if function names match the name of another Python file:
       https://bugzilla.gnome.org/show_bug.cgi?id=709779

   The script simply removes such links. Call it with a single argument - the
   directory containing the documentation HTML files.
"""

import re
import os
import glob
import sys

r = re.compile("""<a [^>]*href="(namespacelink|
                                 namespacecustom__hierarchy|
                                 namespacesetup|
                                 namespaceBallMover)
                  \.html">([^<]*)</a>""", re.VERBOSE)

for f in glob.glob('%s/*.html' % sys.argv[1]):
    outf = open(f + '.out', 'w')
    for line in open(f):
        m = r.search(line)
        outf.write(r.sub(r'\2', line))
    outf.close()
    os.rename(f + '.out', f)
