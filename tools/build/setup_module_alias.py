#!/usr/bin/env python

"""
   Make an alias for the kernel module headers and Python code.
"""

import tools
import os
import glob

def alias_headers(fromdir, todir, incdir):
    for g in glob.glob(os.path.join(fromdir, '*.h')):
        if "Include all non-deprecated headers" not in open(g).read():
            fname = os.path.basename(g)
            contents = """
#include <IMP/kernel_config.h>

IMPKERNEL_DEPRECATED_HEADER(2.5, "Use top-level IMP namespace directly");
#include <%(incdir)s/%(fname)s>
""" % locals()
            tools.rewrite(os.path.join(todir, fname), contents)

def main():
    alias_headers(os.path.join('include', 'IMP'),
                  os.path.join('include', 'IMP', 'kernel'), 'IMP')
    alias_headers(os.path.join('include', 'IMP', 'internal'),
                  os.path.join('include', 'IMP', 'kernel', 'internal'),
                 'IMP/internal')
    tools.link(os.path.join('include', 'IMP.h'),
               os.path.join('include', 'IMP', 'kernel.h'))
    pymod = os.path.join('lib', 'IMP', 'kernel', '__init__.py')
    with open(pymod, 'w') as fh:
        fh.write("""import sys
sys.sysderr.write('IMP.kernel is deprecated - use "import IMP" instead\n')
from IMP import *
""")

if __name__ == '__main__':
    main()
