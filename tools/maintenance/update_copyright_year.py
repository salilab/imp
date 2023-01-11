#!/usr/bin/python3

"""
Update the year range in the IMP copyright notice. This is displayed in
the documentation and by command line tools. Should be run before the
first stable release in a given year.

Note that each .cpp and .h file also has a copyright notice which includes
a year range, but we no longer update those in bulk each year, as these
updates muddy the git logs and provide no meaningful copyright protection.
Instead, we update individual files ad hoc, only when significant changes
are made.
"""

import datetime
import re


FILES = ["README.md", "doc/manual/licenses.md", "tools/debian/copyright",
         "modules/kernel/src/base_utility.cpp"]
YEAR = datetime.datetime.now().year


def patch_file(f):
    with open(f) as fh:
        old_contents = fh.read()
    new_contents = re.sub(r"Copyright 2007\-\d+", "Copyright 2007-%d" % YEAR,
                          old_contents)
    if new_contents == old_contents:
        print("WARNING: nothing substituted in " + f)
    else:
        with open(f, 'w') as fh:
            fh.write(new_contents)


def main():
    for f in FILES:
        patch_file(f)


if __name__ == '__main__':
    main()
