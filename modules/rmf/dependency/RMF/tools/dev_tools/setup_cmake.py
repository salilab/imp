#!/usr/bin/env python

"""Build Files.cmake listing source files for each src, test, examples,
   benchmark directory in the subtree."""

import sys
import glob
import os
import os.path
import difflib

sys.path.append(os.path.split(sys.argv[0])[0])
import python_tools


def _get_files(ds, suffix):
    ret = []
    for (dirpath, dirnames, filenames) in os.walk(ds):
        reldirpath = dirpath[len(ds) + 1:]
        for f in filenames:
            if f.endswith(suffix) and not f.startswith("_"):
                joined = os.path.join(reldirpath, f)
                ret.append(joined.replace("\\", "/"))
    ret.sort()
    return ret


def make_files(d):
    output = os.path.join(d, "Files.cmake")
    cppfiles = _get_files(d, ".cpp")
    cudafiles = _get_files(d, ".cu")
    pyfiles = _get_files(d, ".py")
    jsonfiles = _get_files(d, ".json")
    out = ["set(pyfiles \"%s\")" % ";".join(pyfiles),
           "set(cppfiles \"%s\")" % ";".join(cppfiles),
           "set(cudafiles \"%s\")" % ";".join(cudafiles)]
    if len(jsonfiles) > 0:
        out.append("set(jsonfiles \"%s\")" % ";".join(jsonfiles))
    python_tools.rewrite(output, "\n".join(out) + "\n")


def search_start(d):
    for cd in [o for o in os.listdir(d) if os.path.isdir(os.path.join(d, o))]:
        if cd.startswith("."):
            continue
        cp = os.path.join(d, cd)
        if cd in ["bin", "src", "test", "examples", "benchmark", "utility"]:
            make_files(cp)
        elif not cp.endswith('gitflow'):
            search_start(cp)

search_start(".")
