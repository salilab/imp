"""Simple scanner for Python (.py) files. This is useful if their imports
   are auto-generated in some way (e.g. staged into a build directory, or
   using a tool such as SWIG.)

   Note that different parts of the build 'use' Python files in different ways.
   For example, the unit tests run the Python files, so any imports must exist
   before the files can be used. On the other hand, the documentation merely
   includes the Python files as text (not running them) and so does not need
   the imports to be satisfied. Thus, this scanner is not added to the
   environment's SCANNERS variable, but is added on a per-builder basis using
   the Builder source_scanner argument.

   Note: right now the scan for imports is simplistic and could be caught out
   by 'import' within a multi-line string. Should really use the tokenize
   module to avoid this.
"""

from SCons.Script import Scanner, Dir, FindPathDirs, File
import SCons.Node.FS
import bug_fixes
import data
import os
import re

import_re = re.compile('\s*import\s+(.*?)(\s+as\s+.+)?\s*$')
from_re = re.compile('\s*from\s+(\S+)\s+import\s+(.*?)(\s+as\s+.+)?\s*$')

def _find_python_module(env, modname, dirs):
    """Given a Python module name of the form a.b, return a list of Nodes
       for the actual Python files at each level of the hierarchy
       (e.g. a/__init__.py, a/b/__init.py)."""
    if env['wine'] or env['PLATFORM'] == 'win32':
        suffix='.pyd'
    elif env['PLATFORM'] == 'darwin':
        suffix=".so"
    else:
        suffix=env.subst(env['LDMODULESUFFIX'])
    ret=[]
    if modname == 'IMP':
        nm='kernel'
    else:
        nm=modname[modname.find('.')+1:]
    if data.get(env).modules.has_key(nm):
        if nm=='kernel':
            ret+= ["#/build/lib/_IMP"+suffix,
                   "#/build/lib/IMP/__init__.py"]
        else:
            ret+= ["#/build/lib/_IMP_"+nm+suffix,
                   "#/build/lib/IMP/"+nm+"/__init__.py"]
        ret+= [x.abspath for x in data.get(env).modules[nm].data]
    return ret

def _scanfile(node, env, path):
    # If file does not yet exist, we cannot scan it:
    #print "scanning", node.abspath
    node=bug_fixes.fix_node(env, node)
    if not os.path.exists(node.abspath):
        return []
    # Get directory of input file (for relative imports)
    dir = os.path.dirname(node.path)
    dirs = FindPathDirs('PYTHONPATH')(env) + (Dir(dir),)
    modules = []
    for line in file(node.path, 'r'):
        # Parse lines of the form 'import a.b, a.c (as foo)'
        m = import_re.match(line)
        if m:
            for modname in [x.strip() for x in m.group(1).split(',')]:
                modules.extend(_find_python_module(env, modname, dirs))
        # Parse lines of the form 'from a import b, c (as foo)'
        m = from_re.match(line)
        if m:
            for modname in [x.strip() for x in m.group(2).split(',')]:
                modules.extend(_find_python_module(env,
                                                   m.group(1) + '.' + modname,
                                                   dirs))
    return modules

PythonScanner = Scanner(function=_scanfile, skeys=['.py'], recursive=True)
