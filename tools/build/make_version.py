#!/usr/bin/env python

import subprocess
import tools
from argparse import ArgumentParser
import os


def main():
    parser = ArgumentParser()
    parser.add_argument("--build_dir", help="IMP build directory",
                        default=None)
    parser.add_argument("--module_name", help="Module name", default=None)
    parser.add_argument("-s", "--source", dest="source",
                        help="IMP source directory.")
    args = parser.parse_args()

    # Top-level version
    make_version(args.source, '.')

    # Submodule versions
    mf = tools.ModulesFinder(source_dir=args.source,
                             external_dir=args.build_dir,
                             module_name=args.module_name)
    all_modules = [x for x in mf.values() if isinstance(x, tools.SourceModule)]
    for module in all_modules:
        if os.path.exists(os.path.join(module.path, ".git")):
            make_version(module.path, os.path.join("modules", module.name))


def get_short_rev(source):
    process = subprocess.Popen(['git', 'rev-parse', '--short', 'HEAD'],
                               cwd=source, stdout=subprocess.PIPE,
                               universal_newlines=True)
    version, err = process.communicate()
    return version


def make_version(source, bindir):
    forced = os.path.join(source, "VERSION")
    if os.path.exists(forced):
        with open(forced, "r") as fh:
            version = fh.read()
    elif os.path.exists(os.path.join(source, '.git')):
        process = subprocess.Popen(
            ['git', 'rev-parse', '--abbrev-ref', 'HEAD'],
            cwd=source, stdout=subprocess.PIPE, universal_newlines=True)
        branch, err = process.communicate()
        branch = branch.strip()

        if branch == "develop" or branch.startswith("feature"):
            version = branch + "-" + get_short_rev(source)
        elif branch == "master" or branch.startswith("release"):
            process = subprocess.Popen(['git', 'describe'], cwd=source,
                                       stdout=subprocess.PIPE,
                                       stderr=subprocess.PIPE,
                                       universal_newlines=True)
            version, err = process.communicate()
            if err:
                version = branch + "-" + get_short_rev(source)
        else:
            version = get_short_rev(source)
    else:
        version = 'unknown'

    tools.rewrite(os.path.join(bindir, "VERSION"), version)


if __name__ == '__main__':
    main()
