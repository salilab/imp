#!/usr/bin/python
import sys
from optparse import OptionParser
import os.path
import _tools



parser = OptionParser()
parser.add_option("-n", "--name",
                  dest="name", help="The name of the module.")
parser.add_option("-s", "--swig",
                  dest="swig", help="The name of the swig command.")
parser.add_option("-b", "--build_system",
                  dest="build_system", help="The build system being used.")

def main():
    (options, args) = parser.parse_args()
    cmd= "%s -MM -Iinclude -Iswig swig/IMP_%s.i > src/%s_swig.deps.in"%(options.swig,
                                                                      options.name,
                                                                      options.name)
    print cmd
    os.system(cmd)
    lines= open("swig/%s_swig.deps.in"%options.name, "r").readlines()
    names= [x[:-2].strip() for x in lines[1:]]

    if options.build_system=="scons":
        final_names=["#/build/"+x for x in names]
    else:
        final_names= ["${PROJECT_BINARY_DIR}/%s"%x for x in names]
    open("src/%s_swig.deps"%options.name, "w").write("\n".join(final_names))



if __name__ == '__main__':
    main()
