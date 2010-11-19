"""Simple configure checks for graphviz"""
import os
import sys



def generate(env):
    """Add Builders and construction variables for cpp to an Environment."""
    env['CPP']              = env.WhereIs('cpp')

def exists(env):
    return env.Detect(['cpp'])
