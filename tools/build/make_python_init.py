#!/usr/bin/env python

"""Create the top_level __init__.py for a Python-only module"""

import os.path
import sys
import tools
import shutil
from optparse import OptionParser

def build_init(module, source, infname, outfname):
    version = tools.get_module_version(module.name, source)
    with open(outfname, 'w') as outf:
        if os.path.exists(infname):
            with open(infname) as inf:
                shutil.copyfileobj(inf, outf)

        # Add standard module functions
        outf.write("""

def get_module_version():
    '''Return the version of this module, as a string'''
    return "%s"

def get_module_name():
    '''Return the fully-qualified name of this module'''
    return "IMP::%s"

def get_data_path(fname):
    '''Return the full path to one of this module's data files'''
    return IMP._get_module_data_path("%s", fname)

def get_example_path(fname):
    '''Return the full path to one of this module's example files'''
    return IMP._get_module_example_path("%s", fname)
""" % (version, module.name, module.name, module.name))


def main():
    parser = OptionParser()
    parser.add_option("--build_dir", help="IMP build directory", default=None)
    parser.add_option("-s", "--source",
                      dest="source", help="Where to find IMP source.")

    options, args = parser.parse_args()
    if len(args) != 1:
        parser.error("You must specify an IMP module")
    module, = args

    mf = tools.ModulesFinder(source_dir=options.source,
                             external_dir=options.build_dir,
                             module_name=module,
                             configured_dir="build_info")
    module = mf[module]
    build_init(module, options.source,
            os.path.join(module.path, "pyext", "src", "__init__.py"),
            os.path.join("lib", "IMP", module.name, '__init__.py'))

if __name__ == '__main__':
    main()
