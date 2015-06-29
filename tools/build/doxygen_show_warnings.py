#!/usr/bin/env python

from optparse import OptionParser
import sys

parser = OptionParser()
parser.add_option("-w", "--warnings", dest="warnings",
                  help="The warnings file.")

suppress_strings = ["not generated, too many nodes",
                    "introduction_values",
                    "impligand_score",
                    "Duplicate anchor graphs found",
                    "Duplicate anchor values found",
                    "Duplicate anchor perf found",
                    "impmultifit_tools"]


def main():
    (options, args) = parser.parse_args()
    input = open(options.warnings, "r").readlines()
    found = False
    for l in input:
        # Skip any blank lines that follow a suppressed string
        if found and len(l.strip()) == 0:
            continue
        found = False
        for x in suppress_strings:
            if l.find(x) != -1:
                found = True
                break
        if not found:
            sys.stdout.write(l)

if __name__ == '__main__':
    main()
