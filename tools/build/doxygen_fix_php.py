#!/usr/bin/env python

"""Fix doxygen PHP that doesn't work with PHP 7:

   - the <script> tag is no longer used for PHP code; replace with <?php

   Call this script with a single argument - the
   directory containing the documentation HTML files.
"""

import re
import os
import glob
import sys

r = re.compile(r'<script\s*language="php">', re.IGNORECASE)

for f in glob.glob('%s/*.php' % sys.argv[1]):
    in_script = False
    with open(f + '.out', 'w') as out_f:
        with open(f) as in_f:
            for line in in_f:
                if r.match(line):
                    out = "<?php\n"
                    in_script = True
                elif in_script and line.startswith('</script>'):
                    # Note that this won't handle nested <script> tags
                    in_script = False
                    out = "?>\n";
                else:
                    out = line
                out_f.write(out)
    os.rename(f + '.out', f)
