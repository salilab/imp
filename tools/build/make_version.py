#!/usr/bin/env python

import subprocess
import tools
from optparse import OptionParser
import os


def main():
    parser = OptionParser()
    parser.add_option("--build_dir", help="IMP build directory", default=None)
    parser.add_option("--module_name", help="Module name", default=None)
    parser.add_option("-s", "--source", dest="source",
                      help="IMP source directory.")
    options, args = parser.parse_args()

    # Top-level version
    make_version(options.source, '.')

    # Submodule versions
    mf = tools.ModulesFinder(source_dir=options.source,
                             external_dir=options.build_dir,
                             module_name=options.module_name)
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
