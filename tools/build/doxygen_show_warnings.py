#!/usr/bin/env python

from optparse import OptionParser

parser = OptionParser()
parser.add_option("-w", "--warnings", dest="warnings",
                  help="The warnings file.")

def main():
    (options, args) = parser.parse_args()
    input = open(options.warnings, "r").readlines()
    for l in input:
        if l.find("not generated, too many nodes.") == -1:
            print l


if __name__ == '__main__':
    main()
