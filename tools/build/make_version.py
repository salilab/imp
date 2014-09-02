#!/usr/bin/python

import subprocess
import tools
from optparse import OptionParser
import os


def main():
    parser = OptionParser()
    parser.add_option("-s", "--source", dest="source",
                      help="IMP source directory.")
    options, args = parser.parse_args()

    # Top-level version
    make_version(options.source, '.')

    # Submodule versions
    for module, module_source in tools.get_modules(options.source):
        if os.path.exists(os.path.join(module_source, ".git")):
            make_version(module_source, os.path.join("modules", module))


def get_short_rev(source):
    process = subprocess.Popen(['git', 'rev-parse', '--short', 'HEAD'],
                               cwd=source, stdout=subprocess.PIPE)
    version, err = process.communicate()
    return version


def make_version(source, bindir):
    forced = os.path.join(source, "VERSION")
    if os.path.exists(forced):
        version = open(forced, "r").read()
    else:
        process = subprocess.Popen(
            ['git', 'rev-parse', '--abbrev-ref', 'HEAD'],
            cwd=source, stdout=subprocess.PIPE)
        branch, err = process.communicate()
        branch = branch.strip()

        if branch == "develop" or branch.startswith("feature"):
            version = branch + "-" + get_short_rev(source)
        elif branch == "master" or branch.startswith("release"):
            process = subprocess.Popen(['git', 'describe'], cwd=source,
                                       stdout=subprocess.PIPE,
                                       stderr=subprocess.PIPE)
            version, err = process.communicate()
            if err:
                version = branch + "-" + get_short_rev(source)
        else:
            version = get_short_rev(source)

    if version:
        tools.rewrite(os.path.join(bindir, "VERSION"), version)

if __name__ == '__main__':
    main()
