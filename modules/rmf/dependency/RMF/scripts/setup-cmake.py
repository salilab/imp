#!/usr/bin/env python
import sys
import glob
import os
import os.path
import difflib


def _rewrite(filename, contents, verbose=True):
    try:
        old= open(filename, "r").read()
        if old == contents:
            return
        elif verbose:
            print "    Different", filename
            for l in difflib.unified_diff(old.split("\n"), contents.split("\n")):
                stl= str(l)
                if (stl[0]=='-' or stl[0]=='+') and stl[1] != '-' and stl[1] != '+':
                    print "    "+stl
    except:
        pass
        #print "Missing", filename
    open(filename, "w").write(contents)


def make_all_rmf_header():
  pat= os.path.join("include", "RMF", "*.h")
  allh= glob.glob(pat)
  allh.sort()

  out = []
  for g in allh:
    name= os.path.split(g)[1]
    if name == "HDF5.h":
      continue
    out.append("#include <RMF/" + name + ">")
  # a bit icky
  for d in ["alias_decorators.h",
            "publication_decorators.h",
            "external_decorators.h",
            "sequence_decorators.h",
            "feature_decorators.h",
            "shape_decorators.h",
            "physics_decorators.h"]:
    out.append("#include <RMF/%s>"%d)
  _rewrite(os.path.join("include", "RMF.h"), "\n".join(out)+"\n")

def make_all_hdf5_header():
  pat= os.path.join("include", "RMF", "HDF5", "*.h")
  allh= glob.glob(pat)
  allh.sort()

  out = []
  for g in allh:
    name= os.path.split(g)[1]
    out.append("#include <RMF/HDF5/" + name + ">")
  _rewrite(os.path.join("include", "RMF", "HDF5.h"), "\n".join(out)+"\n")

def make_source_list():
  all = []
  for p in [os.path.join("src"),
            os.path.join("src", "internal"),
            os.path.join("src", "backend", "avro"),
            os.path.join("src", "backend", "hdf5")]:
    cur = glob.glob(os.path.join(p, "*.cpp"))
    cur.sort()
    # ick
    #if p.endswith("avro"):
    #  avros = glob.glob(os.path.join(p, "AvroCPP", "impl", "*.cc")) \
#+ glob.glob(os.path.join(p, "AvroCPP", "impl", "*", "*.cc"))
    #avros.sort()
    #  avros = [x for x in avros if not x.endswith("avrogencpp.cc")]
    #  cur.extend(avros)
    sources = [x.replace("\\", "/") for x in cur]
    _rewrite(os.path.join(p, "Files.cmake"),
             "set(sources ${sources} %s)"%"\n".join(["${PROJECT_SOURCE_DIR}/%s"%x for x in cur])+"\n")

def make_py_test_lists():
  tests = glob.glob(os.path.join("test", "test_*.py"))
  tests.sort()
  _rewrite(os.path.join("test", "PyTests.cmake"), "set(python_tests %s)"%"\n".join(["${PROJECT_SOURCE_DIR}/%s"%x.replace("\\", "/") for x in tests]))

def make_cpp_test_lists():
  tests = glob.glob(os.path.join("test", "test_*.cpp"))
  tests.sort()
  _rewrite(os.path.join("test", "CppTests.cmake"), "set(cpp_tests %s)"%"\n".join(["${PROJECT_SOURCE_DIR}/%s"%x.replace("\\", "/") for x in tests]))

make_all_rmf_header()
make_all_hdf5_header()
make_source_list()
make_py_test_lists()
make_cpp_test_lists()
