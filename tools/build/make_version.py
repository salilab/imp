#!/usr/bin/python

import subprocess
import tools
from optparse import OptionParser
import os


parser = OptionParser()
parser.add_option("-s", "--source", dest="source",
                  help="IMP source directory.")
(options, args) = parser.parse_args()

forced = os.path.join(options.source, "VERSION")
if os.path.exists(forced):
    version = open(forced, "r").read()
else:
    process = subprocess.Popen(['git', 'rev-parse', '--abbrev-ref', 'HEAD'], cwd = options.source, stdout=subprocess.PIPE)
    branch, err = process.communicate()
    branch = branch.strip()

    if branch == "develop" or branch.startswith("feature"):
        process = subprocess.Popen(['git', 'rev-parse', '--short', 'HEAD'], cwd = options.source, stdout=subprocess.PIPE)
        hsh, err = process.communicate()
        version = branch + "-" + hsh
    elif branch == "master" or branch.startswith("release"):
        process = subprocess.Popen(['git', 'describe'], cwd = options.source, stdout=subprocess.PIPE)
        version, err = process.communicate()
    else:
        process = subprocess.Popen(['git', 'rev-parse', '--short', 'HEAD'], cwd = options.source, stdout=subprocess.PIPE)
        version, err = process.communicate()

tools.rewrite("VERSION", version)
