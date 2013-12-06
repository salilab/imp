#!/usr/bin/env python

from optparse import OptionParser

parser = OptionParser()
parser.add_option("-w", "--warnings", dest="warnings",
                  help="The warnings file.")

suppress_strings = ["not generated, too many nodes",
                    "introduction_values",
                    "impligand_score",
                    "impmultifit_tools"]


def main():
    (options, args) = parser.parse_args()
    input = open(options.warnings, "r").readlines()
    for l in input:
        found = False
        for x in suppress_strings:
            if l.find(x) != -1:
                found = True
                break
        if not found:
            print l


if __name__ == '__main__':
    main()
