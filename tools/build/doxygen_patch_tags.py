#!/usr/bin/env python

from optparse import OptionParser

parser = OptionParser()
parser.add_option("-m", "--module", dest="module",
                  help="The module name.")
parser.add_option("-f", "--file", dest="file",
                  help="The tags file.")


def main():
    (options, args) = parser.parse_args()
    with open(options.file, "r") as fh:
        input = fh.read()
    input = input.replace(
        "<name>index</name>",
        "<name>IMP.%s</name>" %
        options.module)
    input = input.replace(
        "<title></title>",
        "<title>IMP.%s</title>" %
        options.module)
    with open(options.file, "w") as fh:
        fh.write(input)


if __name__ == '__main__':
    main()
