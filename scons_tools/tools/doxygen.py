"""Simple configure checks for graphviz"""
import os
import sys


def _get_doxygen_version(env):
    """Run the doxygen command line tool to get and return the version number"""
    if not env['DOXYGEN']:
        return ""
    out = os.popen(env['DOXYGEN'] + ' --version').read()
    version = tuple([int(x) for x in out.split('.')])
    return version

def generate(env):
    """Add Builders and construction variables for doxygen to an Environment."""
    env['DOXYGEN']              = env.WhereIs('doxygen')
    env['DOXYGENVERSION']       = _get_doxygen_version(env)

def exists(env):
    return env.Detect(['doxygen'])
