#!/usr/bin/env python

from argparse import ArgumentParser

parser = ArgumentParser()
parser.add_argument("-m", "--module", dest="module",
                    help="The module name.")
parser.add_argument("-f", "--file", dest="file",
                    help="The tags file.")


def main():
    args = parser.parse_args()
    with open(args.file, "r") as fh:
        input = fh.read()
    input = input.replace(
        "<name>index</name>",
        "<name>IMP.%s</name>" %
        args.module)
    input = input.replace(
        "<title></title>",
        "<title>IMP.%s</title>" %
        args.module)
    with open(args.file, "w") as fh:
        fh.write(input)


if __name__ == '__main__':
    main()
