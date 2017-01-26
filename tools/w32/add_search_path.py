#!/usr/bin/env python

"""Patch IMP and RMF SWIG wrappers to search for Python extensions and DLLs
   in Python version-specific directories. These directories are created by
   the .exe Windows installers, and are not in the standard Python search
   path, so need to be added. We need to patch IMP/__init__.py so we add
   paths before any usage of any IMP module, and RMF.py too in case RMF
   is imported before IMP.

   Note that we used to simply use the 'patch' utility to do this, but the
   SWIG folks changed the header of their output files, which confused patch.
   Instead, we look for import lines, and add our code after the first block
   of imports (which import standard Python modules such as 'sys').
   This ensures that the search path is properly set up before we try to
   import IMP/RMF extensions, but doesn't come before the comment header
   or any __future__ imports (which must come first).
"""

from __future__ import print_function
import re
import sys

IMP_PATCH = r"""
def _add_pyext_to_path():
    import os.path
    import sys
    # Get directory containing IMP's __init__.py
    imp_dir = os.path.abspath(os.path.dirname(__file__))
    # Make sure we're on a Windows system
    if hasattr(sys, 'dllhandle') and len(imp_dir) > 4:
        # Strip '\IMP' suffix from directory
        pydir = imp_dir[:-4]
        # Add Python version-specific directory to search path
        pyextdir = pydir + '\\python%d.%d' % sys.version_info[:2]
        if pyextdir not in sys.path:
            sys.path.insert(1, pyextdir)
        # Strip '\python\IMP' suffix to get directory containing DLLs
        dlldir = imp_dir[:-11] + '\\bin'
        # Add DLL directory to PATH so Windows can find them
        if dlldir not in os.environ['PATH']:
            os.environ['PATH'] = dlldir + ';' + os.environ['PATH']
_add_pyext_to_path()

"""

RMF_PATCH = r"""
def _add_pyext_to_path():
    import os.path
    import sys
    # Get directory containing RMF.py
    rmf_dir = os.path.abspath(os.path.dirname(__file__))
    # Make sure we're on a Windows system
    if hasattr(sys, 'dllhandle') and len(rmf_dir) > 4:
        # Add Python version-specific directory to search path
        pyextdir = rmf_dir + '\\python%d.%d' % sys.version_info[:2]
        if pyextdir not in sys.path:
            sys.path.insert(1, pyextdir)
        # Strip '\python' suffix to get directory containing DLLs
        dlldir = rmf_dir[:-7] + '\\bin'
        # Add DLL directory to PATH so Windows can find them
        if dlldir not in os.environ['PATH']:
            os.environ['PATH'] = dlldir + ';' + os.environ['PATH']
_add_pyext_to_path()

"""

def add_search_path(filename):
    patch = RMF_PATCH if 'RMF' in filename else IMP_PATCH
    with open(filename) as fh:
        contents = fh.readlines()
    # An 'import block' is considered to be a set of lines beginning with
    # 'from' or 'import' statements. Any blank lines or comments are considered
    # to be part of the block.
    r = re.compile('(from|import) ')
    non_statement = re.compile('(\s*$|\s*#)')
    in_imports = False
    imports_done = False
    with open(filename, 'w') as fh:
        for line in contents:
            if not imports_done:
                if not in_imports and r.match(line):
                    in_imports = True
                elif in_imports and not r.match(line) \
                     and not non_statement.match(line):
                    fh.write(patch)
                    in_imports = False
                    imports_done = True
            fh.write(line)

def main():
    for fname in sys.argv[1:]:
        add_search_path(fname)

if __name__ == '__main__':
    main()
