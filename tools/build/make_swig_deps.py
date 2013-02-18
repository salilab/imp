#!/usr/bin/python

""" Compute the dependencies of a swig file by running swig.
"""

import sys
from optparse import OptionParser
import os.path
import tools



parser = OptionParser()
parser.add_option("-n", "--name",
                  dest="name", help="The name of the module.")
parser.add_option("-s", "--swig",
                  dest="swig", default="swig", help="The name of the swig command.")
parser.add_option("-p", "--swigpath",
                  dest="swigpath", help="The swig include path.")
parser.add_option("-i", "--includepath",
                  dest="includepath", help="The incluepath.")
parser.add_option("-b", "--build_system",
                  dest="build_system", help="The build system being used.")

def _fix(name, bs):
    if name.startswith("/"):
        return name
    elif bs=="scons":
        return "#/build/"+name
    else:
        return os.path.join(os.getcwd(), "%s")%name

def main():
    (options, args) = parser.parse_args()
    info = tools.get_module_info(options.name, "/")
    if not info["ok"]:
        tools.rewrite("src/%s_swig.deps"%options.name, "")
        return
    #
    cmd= "%s -MM -Iinclude -Iswig -ignoremissing %s %s swig/IMP_%s.i > src/%s_swig.deps.in"%(options.swig,
" ".join(["-I"+x for x in tools.split(options.swigpath)]),
" ".join(["-I"+x for x in tools.split(options.includepath)]),
                                                                           options.name,
        options.name)
        #print cmd
    #print cmd
    os.system(cmd)
    lines= open("src/%s_swig.deps.in"%options.name, "r").readlines()
    names= [x[:-2].strip() for x in lines[1:]]

    final_names=[_fix(x, options.build_system) for x in names]
    final_list= "\n".join(final_names)
    tools.rewrite("src/%s_swig.deps"%options.name, final_list)



if __name__ == '__main__':
    main()
