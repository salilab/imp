"""Simple configure checks for graphviz"""
import os
import sys



def generate(env):
    """Add Builders and construction variables for dot to an Environment."""
    env['DOT']              = env.WhereIs('dot')

def exists(env):
    return env.Detect(['dot'])
