from SCons.Script import Glob, Dir, File, Builder, Action, Exit, Scanner, Move
import SCons
import os
import sys
import re


def _get_swig_version(env):
    """Run the SWIG command line tool to get and return the version number"""
    if not env['SWIG']:
        return ""
    out = os.popen(env['SWIG'] + ' -version').read()
    match = re.search(r'SWIG Version\s+(\S+)$', out, re.MULTILINE)
    if match:
        return match.group(1)
    else:
        return ""

def generate(env):
    """Add Builders and construction variables for swig to an Environment."""
    env['SWIG']              = env.WhereIs('swig')
    env['SWIGVERSION']       = _get_swig_version(env)

def exists(env):
    return env.Detect(['swig'])
