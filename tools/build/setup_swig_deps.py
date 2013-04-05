#!/usr/bin/env python

""" Compute the dependencies of a swig file by running swig.
"""

import sys
from optparse import OptionParser
import os.path
import tools
import subprocess
import thread_pool



parser = OptionParser()
parser.add_option("-s", "--swig",
                  dest="swig", default="swig", help="The name of the swig command.")
parser.add_option("-b", "--build_system",
                  dest="build_system", help="The build system being used.")

def _fix(name, bs):
    if os.path.isabs(name):
        return name
    elif bs=="scons":
        return "#/build/"+name
    else:
        return os.path.join(os.getcwd(), "%s")%name

def get_dep_merged(modules, name, ordered):
    ret=[]
    alldeps=tools.get_all_dependencies(".", modules, "", ordered)
    for d in alldeps:
        info = tools.get_dependency_info(d, ".")
        lst= tools.split(info[name], ';') # cmake lists are semicolon-separated
        ret.extend(lst)
    ret=list(set(ret))
    ret.sort()
    return ret



def setup_one(module, ordered, build_system, swig):
    info = tools.get_module_info(module, "/")
    if not info["ok"]:
        tools.rewrite("src/%s_swig.deps"%module, "")
        return
    includepath = get_dep_merged([module], "includepath", ordered)
    swigpath = get_dep_merged([module], "swigpath", ordered)


    depf= open("src/%s_swig.deps.in"%module, "w")
    cmd= [swig, "-MM", "-Iinclude", "-Iswig", "-ignoremissing"]\
    + ["-I"+x for x in swigpath] + ["-I"+x for x in includepath]\
    + ["swig/IMP_%s.i"%module]

    ret = subprocess.call(cmd, stdout=depf)
    del depf
    if ret != 0:
        raise OSError("subprocess failed with return code %d: %s" \
                      % (ret, str(cmd)))
    lines= open("src/%s_swig.deps.in"%module, "r").readlines()
    names= [x[:-2].strip() for x in lines[1:]]

    final_names=[_fix(x, build_system) for x in names]
    final_list= "\n".join(final_names)
    tools.rewrite("src/%s_swig.deps"%module, final_list)

def main():
    (options, args) = parser.parse_args()
    pool = thread_pool.ThreadPool()
    ordered = tools.get_sorted_order()
    for m in ordered:
        #setup_one(m, ordered, options.build_system, options.swig)
        pool.add_task(setup_one, m, ordered, options.build_system, options.swig)
    pool.wait_completion()




if __name__ == '__main__':
    main()
