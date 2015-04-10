#!/usr/bin/env python
import sys
import os.path
import os
# first is directory, second data, third subnamespace
output_directory = sys.argv[1]
inputs = sys.argv[2:]


def mkdir(path, clean=True):
    if os.path.isdir(path):
        if clean:
            # remove any old links
            for f in glob.glob(os.path.join(path, "*")):
                if os.path.islink(f):
                    os.unlink(f)
            # Remove old lists of Python tests
            for f in glob.glob(os.path.join(path, "*.pytests")):
                os.unlink(f)
        return
    if os.path.isfile(path):
        os.unlink(path)
    os.makedirs(path)


def rewrite(filename, contents):
    try:
        old = open(filename, "r").read()
        if old == contents:
            return
    except:
        pass
        # print "Missing", filename
    dirpath = os.path.split(filename)[0]
    if dirpath != "":
        mkdir(dirpath, False)
    open(filename, "w").write(contents)


def get_string_contents(path):
    data = open(path, "r").read()
    quoted_data = data.replace("\"", "\\\"").split("\n")
    return quoted_data


def get_function_name(path):
    file_name = os.path.split(path)[1]
    name = file_name.replace(".", "_").lower()
    return name


def get_namespace(path):
    prefix = os.path.split(path)[0]
    dirname = os.path.split(prefix)[1]
    return "data_" + dirname


def write_header(paths):
    names = []
    for p in paths:
        names.append((get_namespace(p), get_function_name(p)))
    decls = []
    for n in names:
        decls.extend(["namespace %s {" % n[0],
                      "extern RMFEXPORT std::string %s;" % n[1],
                      "}"])

    header = """#ifndef RMF_EMBED_JSON_H
#define RMF_EMBED_JSON_H
#include <RMF/config.h>
#include <string>
namespace RMF {
%s
}
#endif
""" % "\n".join(decls)
    outpath = os.path.join(output_directory, "embed_jsons.h")
    rewrite(outpath, header)


def write_cpp(paths):
    names = []
    for p in paths:
        names.append((
            get_namespace(p), get_function_name(p), get_string_contents(p)))
    defs = []
    for n in names:
        defs.extend(["namespace %s {" % n[0],
                     "std::string %s = std::string()" % n[1], ])
        for l in n[2]:
            defs.append("     + \"%s\"" % l)
        defs.extend([";", "}"])
    cpp = """#include "embed_jsons.h"
namespace RMF {
%s
}
""" % "\n".join(defs)
    outpath = os.path.join(output_directory, "embed_jsons.cpp")
    rewrite(outpath, cpp)

try:
    os.makedirs(output_directory)
except:
    pass


write_header(inputs)
write_cpp(inputs)
