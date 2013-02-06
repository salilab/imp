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

from SCons.Script import Scanner, Dir, FindPathDirs, File, Glob
import SCons.Node.FS
import bug_fixes
import data
import utility
import os
import re
import paths as stp
import build_tools.tools

import_re = re.compile('\s*import\s+(.*?)(\s+as\s+.+)?\s*$')
from_re = re.compile('\s*from\s+(\S+)\s+import\s+(.*?)(\s+as\s+.+)?\s*$')

def _find_python_module(env, modname, dirs):
    """Given a Python module name of the form a.b, return a list of Nodes
       for the actual Python files at each level of the hierarchy
       (e.g. a/__init__.py, a/b/__init.py)."""
    kernelinfo =build_tools.tools.get_module_info("kernel",
                                                  env.get("datapath", ""),
        Dir("#/build/").abspath)
    if env['wine'] or env['PLATFORM'] == 'win32':
        suffix='.pyd'
    elif env['PLATFORM'] == 'darwin':
        suffix=".so"
    else:
        suffix=env.subst(env['LDMODULESUFFIX'])
    ret=[]
    #print modname
    if modname == 'IMP':
        if not kernelinfo.has_key("external"):
            return [File("#/build/lib/_IMP_kernel"+env["IMP_PYTHON_SO"])]\
                +stp.get_matching_build(env, ["lib/IMP/kernel/*.py"])\
                +stp.get_matching_build(env, ["data/kernel/*"])
        else:
            return []
    elif modname.startswith("IMP."):
        nm=modname[4:]
        if nm.find(".") != -1:
            nm= nm[:nm.find(".")]
        info =build_tools.tools.get_module_info(nm,
                                                  env.get("datapath", ""),
        Dir("#/build/").abspath)
        if not info.has_key("external") \
           and info["ok"]:
            # pull in kernel too
            libf=[File("#/build/lib/_IMP_"+nm+env["IMP_PYTHON_SO"])]
            pyf= stp.get_matching_build(env, ["lib/IMP/"+nm+"/*.py",
                                               "lib/IMP/"+nm+"/*/*.py",
                                               "lib/IMP/"+nm+"/*/*/*.py"])
            df=stp.get_matching_build_files(env, ["data/"+nm+"/*",
                                                    "data/"+nm+"/*/*"])
            ret=pyf+df+libf
            if not kernelinfo.has_key("external"):
                klibf=[File("#/build/lib/_IMP_kernel"+env["IMP_PYTHON_SO"])]
                kpyf=stp.get_matching_build(env, ["lib/IMP/kernel/*.py"])
                kdf=stp.get_matching_build(env, ["data/kernel/*"])
                ret+=kpyf
                ret+=kdf
                ret+=klibf
            return ret
        else:
            return []
    else:
        return []

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
                ret=_find_python_module(env, modname, dirs)
                modules.extend(ret)
        # Parse lines of the form 'from a import b, c (as foo)'
        m = from_re.match(line)
        if m:
            # Check case where b, c are modules (a.b, a.c)
            for modname in [x.strip() for x in m.group(2).split(',')]:
                modules.extend(_find_python_module(env,
                                                   m.group(1) + '.' + modname,
                                                   dirs))
            # Check case where b, c are classes/functions (check module a)
            modules.extend(_find_python_module(env, m.group(1), dirs))
    return modules

PythonScanner = Scanner(function=_scanfile, skeys=['.py'], recursive=True)
