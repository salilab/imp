#!/usr/bin/env python

"""
   Make an alias for the kernel module headers and Python code.
"""

import tools
import os

def main():
    tools.link(os.path.join('include', 'IMP'),
               os.path.join('include', 'IMP', 'kernel'))
    tools.link(os.path.join('include', 'IMP.h'),
               os.path.join('include', 'IMP', 'kernel.h'))
    pymod = os.path.join('lib', 'IMP', 'kernel', '__init__.py')
    with open(pymod, 'w') as fh:
        fh.write("from IMP import *\n")

if __name__ == '__main__':
    main()
