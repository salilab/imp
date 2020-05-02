#!/usr/bin/env python

"""Fix some broken links in the documentation that result from doxygen bugs:

   - doxygen adds broken links from Python source pages (to namespace*.html)
     if function names match the name of another Python file:
       https://bugzilla.gnome.org/show_bug.cgi?id=709779
     This script simply removes such links.
   - doxygen doesn't handle inline image markdown of the form
     ![alt text](img url)
     This script replaces this with an img tag.

   Call this script with a single argument - the
   directory containing the documentation HTML files.
"""

import re
import os
import glob
import sys

r1 = re.compile("""<a [^>]*href="(namespacelink|
                                  namespacecustom__hierarchy|
                                  namespacesetup|
                                  namespacelog|
                                  namespacestd|
                                  namespaceBallMover)
                   \.html">([^<]*)</a>""", re.VERBOSE)
r2 = re.compile("!\[([^]]+)\]\(([^)]+)\)", re.VERBOSE)

for f in glob.glob('%s/*.html' % sys.argv[1]):
    outf = open(f + '.out', 'w')
    for line in open(f):
        out = r1.sub(r'\2', line)
        out = r2.sub(r'<img src="\2" alt="\1">', out)
        outf.write(out)
    outf.close()
    os.rename(f + '.out', f)
