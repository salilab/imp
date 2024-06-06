#!/usr/bin/env python

from argparse import ArgumentParser
import sys

parser = ArgumentParser()
parser.add_argument("-w", "--warnings", dest="warnings",
                    help="The warnings file.")

suppress_strings = ["not generated, too many nodes",
                    "introduction_values",
                    "impligand_score",
                    "Duplicate anchor graphs found",
                    "Duplicate anchor values found",
                    "Duplicate anchor perf found",
                    "impmultifit_tools"]


def main():
    args = parser.parse_args()
    with open(args.warnings, "r") as fh:
        input = fh.readlines()
    found = False
    for line in input:
        # Skip any blank lines that follow a suppressed string
        if found and len(line.strip()) == 0:
            continue
        found = False
        for x in suppress_strings:
            if line.find(x) != -1:
                found = True
                break
        if not found:
            sys.stdout.write(line)


if __name__ == '__main__':
    main()
