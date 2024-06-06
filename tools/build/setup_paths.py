#!/usr/bin/env python

"""Create the paths.cpp file for the kernel so IMP can find data once
it is installed.
"""

from argparse import ArgumentParser
import tools

template = """
#include <IMP/kernel_config.h>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE
 const char *imp_data_path="%(datapath)s";
 const char *imp_example_path="%(examplepath)s";
IMPKERNEL_END_INTERNAL_NAMESPACE
"""

parser = ArgumentParser()
parser.add_argument("-d", "--datapath", dest="datapath",
                    help="The install data path.")
parser.add_argument("-e", "--examplepath", dest="examplepath",
                    help="The install example path.")
parser.add_argument("-o", "--output", dest="output",
                    help="Where to put the file.")


def main():
    args = parser.parse_args()
    data = {}
    data["examplepath"] = args.examplepath
    data["datapath"] = args.datapath
    g = tools.CPPFileGenerator()
    g.write(args.output, template % data)


if __name__ == '__main__':
    main()
