#!/usr/bin/python
import os.path
import glob
import sys
import copy
import _tools

imp_init="""try:
    from kernel import *
except:
    print "no kernel"
"""

def write_module_cpp(m, contents):
    contents.append("""%%{
#include "IMP/%(module)s.h"
#include "IMP/%(module)s/%(module)s_config.h"
%%}
"""%{"module":m})
    if os.path.exists(os.path.join("include", "IMP", m, "internal", "swig.h")):
        contents.append("""
%%{
#include "IMP/%s/internal/swig.h"
%%}
"""%m)
    if os.path.exists(os.path.join("include", "IMP", m, "internal", "swig_helpers.h")):
        contents.append("""
%%{
#include "IMP/%s/internal/swig_helpers.h"
%%}
"""%m)

def write_module_swig(m, source, contents, skip_import=False):
    path= os.path.join(source, "modules", m, "pyext", "include")
    contents.append("""%%include "IMP/%s/%s_config.h" """%(m,m))
    for macro in glob.glob(os.path.join("include","IMP", m, "*_macros.h")):
        contents.append("%%include \"IMP/%s/%s\""%(m, os.path.split(macro)[1]))
    for macro in glob.glob(os.path.join(path, "*.i")):
        contents.append("%%include \"%s\""%(os.path.split(macro)[1]))
    if not skip_import:
        contents.append("%%import \"IMP_%(module)s.i\""%{"module":m})

def build_wrapper(module, module_path, source, sorted, dependencies, info, target):
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

// XCode 4.6
#pragma clang diagnostic ignored "-Wsometimes-uninitialized"

/* SWIG generates long class names with wrappers that use certain Boost classes,
   longer than the 255 character name length for MSVC. This shouldn't affect
   the code, but does result in a lot of warning output, so disable this warning
   for clarity. */
#ifdef _MSC_VER
#pragma warning( disable: 4503 )
#endif

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
    direct_deps= [(x, False) for x in info["required_modules"]]
    direct_deps.extend([(x, True) for x in info["optional_modules"]])
    all_deps=[]
    for d, o in direct_deps:
        all_deps.append((d,o))
        all_deps.extend(dependencies[d])
    all_deps=list(set(all_deps))
    # some duplicates for optional/non-optional
    all_deps.sort(cmp=lambda x,y: cmp(sorted.index(x[0]), sorted.index(y[0])))

    for m,opt in all_deps:
        if opt:
            contents.append("#if IMP_%(THIS_MODULE)s_HAS_IMP_%(MODULE)s"\
                    %{"module":m, "MODULE":m.upper(), "THIS_MODULE":module.upper()})
        write_module_cpp(m, contents)
        if opt:
            contents.append("#endif\n")

    write_module_cpp(module, contents)
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

    for m,opt in all_deps:
        if opt:
            contents.append("#if IMP_%(THIS_MODULE)s_HAS_IMP_%(MODULE)s"\
                    %{"module":m, "MODULE":m.upper(), "THIS_MODULE":module.upper()})
        write_module_swig(m, source, contents)
        if opt:
            contents.append("#endif\n")

    write_module_swig(module, source, contents, True)

    contents.append(open(os.path.join(module_path, "pyext", "swig.i-in"), "r").read())
    # in case the file doesn't end in one
    contents.append("\n")

    # add support variables
    for m in info["optional_modules"]:
        contents.append("""#ifdef IMP_%(MODULE)s_USE_IMP_%(DEPENDENCY)s
%%pythoncode %%{
has_%(dependency)s=True
%%}
#else
%%pythoncode %%{
has_%(dependency)s=False
%%}
#endif
"""%{"MODULE":module.upper(),
     "module":module,
     "DEPENDENCY": m.upper(),
     "dependency": m})

    for m in info["optional_dependencies"]:
        contents.append("""#if IMP_%(MODULE)s_HAS_%(DEPENDENCY)s
%%pythoncode %%{
has_%(dependency)s=True
%%}
#else
%%pythoncode %%{
has_%(dependency)s=False
%%}
#endif
"""%{"MODULE":module.upper(),
     "module":module,
     "DEPENDENCY": m.upper().replace(".", "_"),
     "dependency": m.lower().replace(".", "_")})
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
    _tools.rewrite(target, "\n".join(contents))

def toposort2(data):
    ret=[]
    while True:
        ordered = set([item for item,dep in data.items() if not dep])
        if not ordered:
            break
        ret.extend(sorted(ordered))
        d = {}
        for item,dep in data.items():
            if item not in ordered:
                d[item] = set([x for x in dep if x[0] not in ordered])
        data = d
    return ret

def get_sorted_order_and_dependencies(source):
    data={}
    for m, path in _tools.get_modules(source):
        df= os.path.join(path, "description")
        if not os.path.exists(df):
            continue
        required_modules=""
        optional_modules=""
        exec open(df, "r").read()
        data[m]= set([(x, False) for x in _tools.split(required_modules)]\
                     + [(x, True) for x in _tools.split(optional_modules)])
        # toposort is destructive
    data2=copy.deepcopy(data)
    sorted= toposort2(data)
    for m in sorted:
        direct= data2[m]
        all=[]
        for md, opt in direct:
            all.append((md, opt))
            all.extend([(x[0], x[1] or opt) for x in data2[md]])
        filtered=list(set([x for x in all if not x[1] or (x[0], False) not in all]))
        filtered.sort()
        data2[m]=filtered
    return sorted, data2

def main():
    source=sys.argv[1]
    sorted_order, dependencies=get_sorted_order_and_dependencies(source)
    #print sorted_order
    #print dependencies
    _tools.rewrite("lib/IMP/__init__.py", imp_init)
    for m, path in _tools.get_modules(source):
        build_wrapper(m, path, source, sorted_order,
                      dependencies, _tools.get_module_data(path), os.path.join("swig", "IMP_"+m+".i"))

if __name__ == '__main__':
    main()
