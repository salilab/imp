#!/usr/bin/env python3
"""
Simple script to print all header files where the doxygen \file
comment does not match the name of the file.
"""


import re
import pathlib

p = pathlib.Path('.')
filere = re.compile(r'\\file\s+(\S+)')


def read_name(fname):
    with open(fname) as fh:
        for line in fh:
            m = filere.search(line)
            if m:
                return m.group(1)


for f in p.rglob("*.h"):
    name = read_name(f)
    if name:
        lastname = name.split('/')[-1]
        if lastname != f.name:
            print(f)
