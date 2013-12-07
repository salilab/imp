#!/usr/bin/env python

from optparse import OptionParser

parser = OptionParser()
parser.add_option("-m", "--module", dest="module",
                  help="The module name.")
parser.add_option("-f", "--file", dest="file",
                  help="The tags file.")


def main():
    (options, args) = parser.parse_args()
    input = open(options.file, "r").read()
    input = input.replace(
        "<name>index</name>",
        "<name>IMP.%s</name>" %
        options.module)
    input = input.replace(
        "<title></title>",
        "<title>IMP.%s</title>" %
        options.module)
    open(options.file, "w").write(input)

if __name__ == '__main__':
    main()
