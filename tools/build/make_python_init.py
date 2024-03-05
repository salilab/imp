#!/usr/bin/env python

"""Create the top_level __init__.py for a Python-only module"""

import os.path
import tools
import shutil
from argparse import ArgumentParser


def build_init(module, source, infname, outfname):
    version = tools.get_module_version(module.name, source)
    with open(outfname, 'w') as outf:
        if os.path.exists(infname):
            with open(infname) as inf:
                shutil.copyfileobj(inf, outf)

        # Add standard module functions and constants
        outf.write("""

__version__ = "%s"

def get_module_version():
    '''Return the version of this module, as a string'''
    return "%s"

def get_module_name():
    '''Return the fully-qualified name of this module'''
    return "IMP::%s"

def get_data_path(fname):
    '''Return the full path to one of this module's data files'''
    import IMP
    return IMP._get_module_data_path("%s", fname)

def get_example_path(fname):
    '''Return the full path to one of this module's example files'''
    import IMP
    return IMP._get_module_example_path("%s", fname)
""" % (version, version, module.name, module.name, module.name))


def main():
    parser = ArgumentParser()
    parser.add_argument("--build_dir", help="IMP build directory",
                        default=None)
    parser.add_argument("-s", "--source",
                        dest="source", help="Where to find IMP source.")
    parser.add_argument("module", help="IMP module name")

    args = parser.parse_args()

    mf = tools.ModulesFinder(source_dir=args.source,
                             external_dir=args.build_dir,
                             module_name=args.module)
    module = mf[args.module]
    build_init(
        module, args.source,
        os.path.join(module.path, "pyext", "src", "__init__.py"),
        os.path.join("lib", "IMP", module.name, '__init__.py'))


if __name__ == '__main__':
    main()
