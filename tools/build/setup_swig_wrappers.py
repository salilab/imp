#!/usr/bin/env python

""" Create the swig wrapper files from setup.i-in files.
"""

import os.path
import glob
import sys
import copy
import tools
from optparse import OptionParser


imp_init="""try:
    from kernel import *
except:
    print "no kernel"
"""

def write_module_cpp(m, contents, datapath):
    info= tools.get_module_info(m, datapath)
    contents.append("""%%{
#include "IMP/%(module)s.h"
#include "IMP/%(module)s/%(module)s_config.h"
%%}
"""%{"module":m})
    for macro in info["swig_wrapper_includes"]:
        contents.append("""%%{
#include <%s>
%%}"""%(macro))


def write_module_swig(m, source, contents, datapath, skip_import=False):
    info= tools.get_module_info(m, datapath)
    contents.append("""%%include "IMP/%s/%s_config.h" """%(m,m))
    for macro in info["swig_includes"]:
        contents.append("%%include \"%s\""%(macro))
    if not skip_import:
        contents.append("%%import \"IMP_%(module)s.i\""%{"module":m})

def build_wrapper(module, module_path, source, sorted, info, target, datapath):
    info = tools.get_module_info(module, datapath)
    if not info["ok"]:
        return
    contents=[]
    swig_module_name="IMP."+module

    contents.append("""%%module(directors="1", allprotected="1") "%s"
%%feature("autodoc", 1);
// turn off the warning as it mostly triggers on methods (and lots of them)
%%warnfilter(321);

%%inline %%{
namespace IMP {
namespace kernel {
}
using namespace kernel;
}
%%}

%%{
#include <boost/version.hpp>
#if BOOST_VERSION > 103600
#if BOOST_VERSION > 103800
#include <boost/exception/all.hpp>
#else
#include <boost/exception.hpp>
#endif
#endif

#include <boost/type_traits/is_convertible.hpp>
#include <boost/utility/enable_if.hpp>
#include <exception>

%%}
"""%swig_module_name)
        # some of the typemap code ends up before this is swig sees the typemaps first
    all_deps = [x for x in tools.get_dependent_modules([module], datapath) if x != module]
    for m in reversed(all_deps):
        write_module_cpp(m, contents, datapath)

    write_module_cpp(module, contents, datapath)
    contents.append("""
%implicitconv;
%include "std_vector.i"
%include "std_string.i"
%include "std_pair.i"


%pythoncode %{
_value_types=[]
_object_types=[]
_raii_types=[]
_plural_types=[]
%}

%include "typemaps.i"

#ifdef NDEBUG
#error "The python wrappers must not be built with NDEBUG"
#endif

""")

    for m in reversed(all_deps):
        write_module_swig(m, source, contents, datapath)

    write_module_swig(module, source, contents, datapath, True)

    contents.append("%%include \"IMP_%s.impl.i\""%module)
    #contents.append(open(os.path.join(module_path, "pyext", "swig.i-in"), "r").read())

    contents.append("""
namespace IMP {
namespace %s {
const std::string get_module_version();
std::string get_example_path(std::string fname);
std::string get_data_path(std::string fname);
}
}
"""%module)
    contents.append("""%pythoncode %{
import _version_check
_version_check.check_version(get_module_version())
%}
""")
    tools.rewrite(target, "\n".join(contents))


parser = OptionParser()
parser.add_option("-d", "--datapath",
                  dest="datapath", default="", help="Extra data path.")
parser.add_option("-s", "--source",
                  dest="source", help="Where to find IMP source.")
parser.add_option("-m", "--module",
                  dest="module", default="", help="Only run on one module.")


def main():
    (options, args) = parser.parse_args()
    sorted_order = tools.get_sorted_order()
    if options.module != "":
        if options.module=="kernel":
            tools.rewrite("lib/IMP/__init__.py", imp_init)
        build_wrapper(options.module, os.path.join(options.source, "modules", options.module),
                      options.source, sorted_order,
                      tools.get_module_description(options.source, options.module, options.datapath),
            os.path.join("swig", "IMP_"+options.module+".i"),
            options.datapath)
    else:
        tools.rewrite("lib/IMP/__init__.py", imp_init)
        for m, path in tools.get_modules(options.source):
            build_wrapper(m, path, options.source, sorted_order,
                          tools.get_module_description(options.source, m, options.datapath),
                          os.path.join("swig", "IMP_"+m+".i"),
                          options.datapath)

if __name__ == '__main__':
    main()
