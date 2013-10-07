#!/usr/bin/env python

"""
Set up a module by
- checking that all required modules and dependencies are found
- creating the config header and .cpp and version check .py
- linking .py files from the bin and benchmarks directories into the build dir

If the module cannot be configured, the script exits with an error.
"""

import sys
from optparse import OptionParser
import os.path
import tools
import glob

parser = OptionParser()
parser.add_option("-D", "--defines", dest="defines", default="",
                  help="Colon separated list of defines.")
parser.add_option("-n", "--name",
                  dest="name", help="The name of the module.")
parser.add_option("-s", "--source",
                  dest="source", help="The root for IMP source.")
parser.add_option("-d", "--datapath",
                  dest="datapath", default="", help="An extra IMP datapath.")

def get_version(options):
    module_version= os.path.join(options.source, "modules", options.name, "VERSION")
    if os.path.exists(module_version):
        return open(module_version, "r").read().split("\n")[0]
    else:
        imp_version= os.path.join(options.source, "VERSION")
        if os.path.exists(imp_version):
            return open(imp_version, "r").read().split("\n")[0]
        else:
            return "develop"

def add_list_to_defines(cppdefines, data, sym, val, names):
    names.sort()
    for n in names:
        nn= n.replace(".", "_").upper()
        cppdefines.append("#define IMP_%s_%s_%s"%(data["name"].upper(), sym, nn))
        cppdefines.append("#define IMP_%s_HAS_%s %d"%(data["name"].upper(), nn, val))

def make_header(options):
    dir= os.path.join("include", "IMP", options.name)
    file=os.path.join(dir, "%s_config.h"%options.name)
    header_template = open(os.path.join(options.source, "tools", "build", "config_templates", "header.h"), "r").read()
    try:
        os.makedirs(dir)
    except:
        #exists
        pass

    data={}
    data["name"]= options.name
    data["filename"]="IMP/%s/%s_config.h"%(options.name, options.name)
    data["cppprefix"]="IMP%s"%options.name.upper().replace("_", "")
    if data["name"] !="base":
        data["showable"]="""#if !defined(IMP_DOXYGEN) && !defined(SWIG)

#include <IMP/base/Showable.h>
#include <IMP/base/hash.h>

namespace IMP { namespace %(name)s {
using ::IMP::base::Showable;
using ::IMP::base::operator<<;
using ::IMP::base::hash_value;
} } // namespace
namespace IMP { namespace %(name)s { namespace internal {
using ::IMP::base::Showable;
using ::IMP::base::operator<<;
using ::IMP::base::hash_value;
} } } // namespace

#endif // !defined(SWIG) && !defined(IMP_DOXYGEN)
"""%data
    else:
        data["showable"]=""

    cppdefines=[]
    if options.defines != "":
        for define in tools.split(options.defines):
            parts= define.split("=")
            if len(parts) ==2:
                cppdefines.append("#define %s %s"%(parts[0], parts[1]))
            else:
                cppdefines.append("#define %s"%parts[0])

    required_modules=""
    lib_only_required_modules=""
    required_dependencies=""
    optional_dependencies=""
    exec open(os.path.join(options.source, "modules", data["name"], "dependencies.py"), "r").read()

    info= tools.get_module_info(data["name"], options.datapath)

    optional_modules=[x for x in info["modules"] if x not in tools.split(required_modules) and x != ""]
    unfound_modules=[x for x in info["unfound_modules"] if x != ""]
    optional_dependencies=[x for x in info["dependencies"] if x not in tools.split(required_dependencies) and x != ""]
    unfound_dependencies=[x for x in info["unfound_dependencies"] if x != ""]
    add_list_to_defines(cppdefines, data, "USE", 1,
                        ["imp_"+x for x in optional_modules])
    add_list_to_defines(cppdefines, data, "NO", 0,
                        ["imp_"+x for x in unfound_modules])
    add_list_to_defines(cppdefines, data, "USE", 1, optional_dependencies)
    add_list_to_defines(cppdefines, data, "NO", 0, info["unfound_dependencies"])
    data["cppdefines"]="\n".join(cppdefines)
    tools.rewrite(file, header_template%data)

def make_cpp(options):
    dir= os.path.join("src", options.name)
    file=os.path.join(dir, "config.cpp")
    cpp_template = open(os.path.join(options.source, "tools", "build", "config_templates", "src.cpp"), "r").read()
    try:
        os.makedirs(dir)
    except:
        # exists
        pass
    data={}
    data["filename"]="IMP/%s/%s_config.h"%(options.name, options.name)
    data["cppprefix"]="IMP%s"%options.name.upper().replace("_", "")
    data["name"]= options.name
    data["version"]= get_version(options)
    tools.rewrite(file, cpp_template%data)

def make_doxygen(options, modules):
    file = os.path.join("doxygen", options.name, "Doxyfile")
    name = options.name
    template_file = os.path.join(options.source, "tools", "build", "doxygen_templates", "Doxyfile.in")
    template = open(template_file, "r").read()
    template = template.replace("@IMP_SOURCE_PATH@", options.source)
    template = template.replace("@VERSION@", get_version(options))
    template = template.replace("@NAME@", name)
    template = template.replace("@PROJECT_BRIEF@",
                                '"The Integrative Modeling Platform"')
    template = template.replace("@RECURSIVE@", "YES")
    template = template.replace("@EXCLUDE_PATTERNS@", "*/tutorial/*")
    template = template.replace("@IS_HTML@", "NO")
    template = template.replace("@IS_XML@", "YES")
    template = template.replace("@PROJECT_NAME@", "IMP."+name)
    template = template.replace("@HTML_OUTPUT@", "../../doc/html/" + name)
    template = template.replace("@XML_OUTPUT@", "xml")
    template = template.replace("@GENERATE_TAGFILE@", "tags")
    template = template.replace("@LAYOUT_FILE@",
                      "%s/doc/doxygen/module_layout.xml" % options.source)
    template = template.replace("@MAINPAGE@", "README.md")
    template = template.replace("@INCLUDE_PATH@", "include")
    template = template.replace("@FILE_PATTERNS@", "*.cpp *.h *.py *.md *.dox")
    template = template.replace("@WARNINGS@", "warnings.txt")
    # include lib and doxygen in imput
    inputs = []
    if options.name == "kernel":
        inputs.append("include/IMP/")
        exclude = ["include/IMP/%s include/IMP/%s.h"%(m,m) for m,g in tools.get_modules(options.source) if m != "kernel"]
        template = template.replace("@EXCLUDE@", " \\\n                         ".join(exclude))
    else:
        template = template.replace("@EXCLUDE@", "")
    inputs.append("include/IMP/"+options.name)
    inputs.append("lib/IMP/"+options.name)
    inputs.append("examples/"+options.name)
    # suppress a warning since git removes empty dirs and doxygen gets confused
    # if the input path doesn't exist
    docpath = os.path.join(options.source, "modules", options.name, "doc")
    if os.path.exists(docpath):
        inputs.append(docpath)
    # overview for module
    inputs.append("../generated/IMP_%s.dox"%options.name)
    template = template.replace("@INPUT_PATH@", " \\\n                         ".join(inputs))
    tags = []
    for m in modules:
        tags.append(os.path.join("../", m, "tags") + "=" + "../"+m)
    template = template.replace("@TAGS@", " \\\n                         ".join(tags))
    if options.name == "example":
        template = template.replace("@EXAMPLE_PATH@",
                                    "examples/example %s/modules/example"%options.source)
    else:
        template = template.replace("@EXAMPLE_PATH@", "examples/"+options.name)
    tools.rewrite(file, template)

def write_no_ok(module):
    new_order= [x for x in tools.get_sorted_order() if x != module]
    tools.set_sorted_order(new_order)
    tools.rewrite(os.path.join("data", "build_info", "IMP."+module), "ok=False\n", verbose=False)

def write_ok(module, modules, unfound_modules, dependencies, unfound_dependencies,
             swig_includes, swig_wrapper_includes):
    print "yes"
    config=["ok=True"]
    if len(modules) > 0:
        config.append("modules = \"" + ":".join(modules)+"\"")
    if len(unfound_modules) > 0:
        config.append("unfound_modules = \""+ ":".join(unfound_modules)+"\"")
    if len(dependencies) > 0:
        config.append("dependencies = \"" + ":".join(dependencies)+"\"")
    if len(unfound_dependencies) > 0:
        config.append("unfound_dependencies = \"" + ":".join(unfound_dependencies)+"\"")
    if len(swig_includes) > 0:
        config.append("swig_includes = \"" + ":".join(swig_includes)+"\"")
    if len(swig_wrapper_includes) > 0:
        config.append("swig_wrapper_includes = \"" + ":".join(swig_wrapper_includes)+"\"")
    tools.rewrite(os.path.join("data", "build_info", "IMP."+module), "\n".join(config))

def setup_module(module, source, datapath):
    print "Configuring module", module, "...",
    data= tools.get_module_description(source, module, datapath)
    for d in data["required_dependencies"]:
        if not tools.get_dependency_info(d, datapath)["ok"]:
            print d, "not found"
            write_no_ok(module)
            return False, []
    dependencies = data["required_dependencies"]
    unfound_dependencies = []
    for d in data["optional_dependencies"]:
        if tools.get_dependency_info(d, datapath)["ok"]:
            dependencies.append(d)
        else:
            unfound_dependencies.append(d)
    for d in data["required_modules"]:
        if not tools.get_module_info(d, datapath)["ok"]:
            print "IMP."+d, "not found"
            write_no_ok(module)
            return False, []
    modules= data["required_modules"]
    unfound_modules = []
    for d in data["optional_modules"]:
        if tools.get_module_info(d, datapath)["ok"]:
            modules.append(d)
        else:
            unfound_modules.append(d)
    all_modules=tools.get_dependent_modules(modules, datapath)
    swig_includes=[os.path.split(x)[1] for x
                   in tools.get_glob([os.path.join(source, "modules", module,
                                                   "pyext", "include", "*.i")])]\
                 + ["IMP/"+module+"/"+os.path.split(x)[1] for x
                            in tools.get_glob([os.path.join("include", "IMP", module, "*_macros.h")])]
    swig_wrapper_includes= ["IMP/"+module+"/internal/"+os.path.split(x)[1] for x
                   in tools.get_glob([os.path.join(source, "modules", module, "include", "internal", "swig*.h")])]
    tools.mkdir(os.path.join("src", module))
    tools.mkdir(os.path.join("src", module+"_swig"))
    write_ok(module, all_modules,
             unfound_modules, tools.get_dependent_dependencies(all_modules, dependencies,datapath),
             unfound_dependencies, swig_includes, swig_wrapper_includes)
    return True,all_modules

def link_bin(options):
    path = os.path.join("module_bin", options.name)
    tools.mkdir(path, clean=False)
    for old in tools.get_glob([os.path.join(path, "*.py")]):
        os.unlink(old)
    tools.link_dir(os.path.join(options.source, "modules", options.name, "bin"), path, clean=False, match=["*.py"])

def link_benchmark(options):
    path = os.path.join("benchmark", options.name)
    tools.mkdir(path, clean=False)
    for old in tools.get_glob([os.path.join(path, "*.py")]):
        os.unlink(old)
    tools.link_dir(os.path.join(options.source, "modules", options.name, "benchmark"), path, clean=False, match=["*.py"])

def make_overview(options):
    rmd = open(os.path.join(options.source, "modules", options.name, "README.md"), "r").read()
    tools.rewrite(os.path.join("doxygen", "generated", "IMP_%s.dox" % options.name),
                  """/** \\page imp%s IMP.%s
\\tableofcontents

%s
*/
""" %(options.name, options.name, rmd))


def main():
    (options, args) = parser.parse_args()
    disabled= tools.split(open("data/build_info/disabled", "r").read(), "\n")
    if options.name in disabled:
        print options.name, "is disabled"
        write_no_ok(options.name)
        tools.rmdir(os.path.join("module_bin", options.name))
        tools.rmdir(os.path.join("benchmark", options.name))
        sys.exit(1)
    success, modules = setup_module(options.name, options.source, options.datapath)
    if success:
        make_header(options)
        make_doxygen(options, modules)
        make_overview(options)
        link_bin(options)
        link_benchmark(options)
        sys.exit(0)
    else:
        tools.rmdir(os.path.join("module_bin", options.name))
        tools.rmdir(os.path.join("benchmark", options.name))
        sys.exit(1)

if __name__ == '__main__':
    main()
