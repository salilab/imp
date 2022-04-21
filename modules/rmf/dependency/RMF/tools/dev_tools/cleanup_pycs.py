#!/usr/bin/env python3

"""Traverses the directory tree deleting any .pyc's who do not have a
   source .py. Helpful when switching between revisions with source control."""

from __future__ import print_function
from os.path import join
from os import walk, unlink
import re
import sys

PYTHON_RE = re.compile(R'^(.*\.py)(c)?$')

global count
count = 0

directories = sys.argv[1:] if len(sys.argv) > 1 else ['.']
for directory in directories:
    for (dirpath, dirnames, filenames) in walk(directory):
        source_set = set()
        pyc_set = set()
        for filename in filenames:
            match = PYTHON_RE.match(filename)
            if match is not None:
                (source_set if match.group(2) is None else pyc_set).add(
                    match.group(1))
        to_remove = (pyc_set - source_set)
        for extra in to_remove:
            unlink(join(dirpath, extra + 'c'))
            count += 1

if count > 0:
    print('Removed \033[91m%d\033[0m file%s'
          % (count, 's' if count > 1 else ''))
