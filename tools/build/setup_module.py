#!/usr/bin/env python

"""
Set up a module by
- checking that all required modules and dependencies are found
- creating the config header and .cpp and version check .py
- linking .py files from the bin and benchmarks directories into the build dir
- Linking Python apps into the build dir.

If the module cannot be configured, the script exits with an error.
"""

import sys
from optparse import OptionParser
import os.path
import tools
import glob
import pickle
import re

TOPDIR = os.path.abspath(os.path.dirname(__file__))

parser = OptionParser()
parser.add_option("--build_dir", help="IMP build directory", default=None)
parser.add_option("-D", "--defines", dest="defines", default="",
                  help="Colon separated list of defines.")
parser.add_option("-n", "--name",
                  dest="name", help="The name of the module.")
parser.add_option("-s", "--source",
                  dest="source", help="The root for IMP source.")
parser.add_option("-d", "--datapath",
                  dest="datapath", default="", help="An extra IMP datapath.")


def add_list_to_defines(cppdefines, data, sym, val, names):
    for n in sorted(names):
        nn = n.replace(".", "_").replace("-", "_").upper()
        cppdefines.append("#define IMP_%s_%s_%s"
                          % (data["name"].upper(), sym, nn))
        cppdefines.append("#define IMP_%s_HAS_%s %d"
                          % (data["name"].upper(), nn, val))


def make_header(options, module):
    if module.python_only:
        return
    if module.name == 'kernel':
        dir = os.path.join("include", "IMP")
    else:
        dir = os.path.join("include", "IMP", module.name)
    file = os.path.join(dir, "%s_config.h" % module.name)
    header_template = tools.CPPFileGenerator(
        os.path.join(TOPDIR, "config_templates", "header.h"))
    try:
        os.makedirs(dir)
    except OSError:
        # exists
        pass

    data = {}
    data["name"] = module.name
    if module.name == 'kernel':
        data["namespace"] = "IMP"
        data["begin_ns"] = "namespace IMP{"
        data["end_ns"] = "}"
        data["filename"] = "IMP/%s_config.h" % module.name
    else:
        data["namespace"] = "IMP::%s" % module.name
        data["begin_ns"] = "namespace IMP{ namespace %s {" % module.name
        data["end_ns"] = "} }"
        data["filename"] = "IMP/%s/%s_config.h" % (module.name, module.name)
    data["cppprefix"] = "IMP%s" % module.name.upper().replace("_", "")
    if data["name"] != "kernel":
        data["showable"] = """#if !defined(IMP_DOXYGEN) && !defined(SWIG)

#include <IMP/Showable.h>
#include <IMP/hash.h>

%(begin_ns)s
using ::IMP::Showable;
using ::IMP::operator<<;
using ::IMP::hash_value;
%(end_ns)s // namespace
%(begin_ns)s namespace internal {
using ::IMP::Showable;
using ::IMP::operator<<;
using ::IMP::hash_value;
} %(end_ns)s // namespace

#endif // !defined(SWIG) && !defined(IMP_DOXYGEN)
""" % data
    else:
        data["showable"] = ""
    cppdefines = []
    if options.defines != "":
        for define in tools.split(options.defines):
            parts = define.split("=")
            if len(parts) == 2:
                cppdefines.append("#define %s %s" % (parts[0], parts[1]))
            else:
                cppdefines.append("#define %s" % parts[0])

    cf = module.configured

    optional_modules = [x for x in cf.modules
                        if x not in module.required_modules]
    optional_dependencies = [x for x in cf.dependencies
                             if x not in module.required_dependencies]
    add_list_to_defines(cppdefines, data, "USE", 1,
                        ["imp_" + x.name for x in optional_modules])
    add_list_to_defines(cppdefines, data, "NO", 0,
                        ["imp_" + x.name for x in cf.unfound_modules])
    add_list_to_defines(cppdefines, data, "USE", 1, optional_dependencies)
    add_list_to_defines(cppdefines, data, "NO", 0, cf.unfound_dependencies)
    data["cppdefines"] = "\n".join(cppdefines)
    header_template.write(file, data)


class ModuleDoxFileGenerator(tools.FileGenerator):
    def __init__(self, template_file, module, modules, finder):
        self.module = module
        self.modules = modules
        self.finder = finder
        tools.FileGenerator.__init__(self, template_file, '#')

    def get_output_file_contents(self, options):
        template = self.template
        module = self.module
        modules = self.modules
        template = template.replace("@IMP_SOURCE_PATH@", options.source)
        template = template.replace("@VERSION@", "NONE")
        template = template.replace("@NAME@", module.name)
        template = template.replace("@PROJECT_BRIEF@",
                                    '"The Integrative Modeling Platform"')
        template = template.replace("@RECURSIVE@", "YES")
        template = template.replace("@EXCLUDE_PATTERNS@", "*/tutorial/*")
        template = template.replace("@IS_HTML@", "NO")
        template = template.replace("@IS_XML@", "YES")
        template = template.replace("@PROJECT_NAME@", "IMP." + module.name)
        template = template.replace("@HTML_OUTPUT@", "../../doc/html/"
                                    + module.name)
        template = template.replace("@XML_OUTPUT@", "xml")
        template = template.replace("@TREEVIEW@", "NO")
        template = template.replace("@GENERATE_TAGFILE@", "tags")
        template = template.replace(
            "@LAYOUT_FILE@",
            "%s/doc/doxygen/module_layout.xml" % options.source)
        template = template.replace("@MAINPAGE@", "README.md")
        template = template.replace("@INCLUDE_PATH@", "include")
        template = template.replace("@FILE_PATTERNS@",
                                    "*.cpp *.h *.py *.md *.dox")
        template = template.replace("@WARNINGS@", "warnings.txt")
        # include lib and doxygen in input
        inputs = []
        if module.name == "kernel":
            inputs.append("lib/IMP/")
            inputs.append("include/IMP/")
            exclude = ["include/IMP/%s include/IMP/%s.h lib/IMP/%s"
                       % (m, m, m) for m in sorted(self.finder.keys())]
            exclude.append("include/IMP/base include/IMP/base.h lib/IMP/base")
            template = template.replace(
                "@EXCLUDE@", " \\\n                         ".join(exclude))
        else:
            template = template.replace("@EXCLUDE@", "")
            inputs.append("include/IMP/" + module.name)
            inputs.append("lib/IMP/" + module.name)
        inputs.append("examples/" + module.name)
        # suppress a warning since git removes empty dirs and doxygen
        # gets confused if the input path doesn't exist
        docpath = os.path.join(module.path, "doc")
        if os.path.exists(docpath):
            inputs.append(docpath)
        # overview for module
        inputs.append("../generated/IMP_%s.dox" % module.name)
        template = template.replace(
            "@INPUT_PATH@", " \\\n                         ".join(inputs))
        tags = [os.path.join(options.source, 'doc', 'doxygen',
                             'dummy_module_tags.xml')]
        for m in modules:
            tags.append(os.path.join("../", m.name, "tags")
                        + "=" + "../" + m.name)
        template = template.replace(
            "@TAGS@", " \\\n                         ".join(tags))
        if module.name == "example":
            template = template.replace(
                "@EXAMPLE_PATH@",
                "examples/example %s/modules/example" % options.source)
        else:
            template = template.replace("@EXAMPLE_PATH@",
                                        "examples/" + module.name)
        return template


def make_doxygen(options, module, modules, finder):
    file = os.path.join("doxygen", module.name, "Doxyfile")
    g = ModuleDoxFileGenerator(
        os.path.join(TOPDIR, "doxygen_templates", "Doxyfile.in"),
        module, modules, finder)
    g.write(file, options)


def write_no_ok(module):
    new_order = [x for x in tools.get_sorted_order() if x != module]
    tools.set_sorted_order(new_order)
    tools.rewrite(os.path.join("build_info", "IMP." + module),
                  "ok=False\n", verbose=False)


def write_ok(module, modules, unfound_modules, dependencies,
             unfound_dependencies, swig_includes, swig_wrapper_includes,
             python_only):
    print("yes")
    config = ["ok=True"]
    modules = [x.name for x in modules]
    unfound_modules = [x.name for x in unfound_modules]
    vardict = locals()
    for varname in ("modules", "unfound_modules", "dependencies",
                    "unfound_dependencies", "swig_includes",
                    "swig_wrapper_includes"):
        var = vardict[varname]
        if len(var) > 0:
            config.append("%s = %s" % (varname, repr(":".join(var))))
    if python_only:
        config.append("python_only = True")
    tools.rewrite(os.path.join("build_info", "IMP." + module.name),
                  "\n".join(config))


def setup_module(module, finder):
    sys.stdout.write("Configuring module %s ..." % module.name)
    for d in module.required_dependencies:
        if not finder.get_dependency_info(d)["ok"]:
            print("Required dependency %s not found" % d)
            write_no_ok(module.name)
            return False, []
    dependencies = module.required_dependencies[:]
    unfound_dependencies = []
    for d in module.optional_dependencies:
        if finder.get_dependency_info(d)["ok"]:
            dependencies.append(d)
        else:
            unfound_dependencies.append(d)
    for d in module.required_modules:
        if not d.configured.ok:
            print("Required module IMP.%s not available" % d.name)
            write_no_ok(module.name)
            return False, []
    modules = module.required_modules[:]
    unfound_modules = []
    for d in module.optional_modules:
        if d.configured.ok:
            modules.append(d)
        else:
            unfound_modules.append(d)
    all_modules = finder.get_dependent_modules(modules)
    moddir = os.path.join('IMP',
                          '' if module.name == 'kernel' else module.name)
    swig_includes = \
        [os.path.split(x)[1]
         for x in tools.get_glob([os.path.join(module.path, "pyext",
                                               "include", "*.i")])] \
        + [os.path.join(moddir, os.path.split(x)[1])
           for x in tools.get_glob([os.path.join("include", moddir,
                                                 "*_macros.h")])]
    swig_wrapper_includes = [os.path.join(moddir, "internal",
                                          os.path.split(x)[1])
                             for x in tools.get_glob(
                                 [os.path.join(module.path,
                                               "include", "internal",
                                               "swig*.h")])]
    tools.mkdir(os.path.join("src", module.name))
    tools.mkdir(os.path.join("src", module.name + "_swig"))
    write_ok(module, all_modules, unfound_modules,
             finder.get_dependent_dependencies(all_modules, dependencies),
             unfound_dependencies, swig_includes, swig_wrapper_includes,
             module.python_only)
    return True, all_modules


def link_py_apps(module):
    path = os.path.join(module.path, "bin")
    tools.mkdir("bin", clean=False)
    bins = [b for b in glob.glob(os.path.join(path, '*'))
            if tools.filter_pyapps(b)]
    # rewrite Python shebang to use current version of Python (2 or 3)
    for source_bin in bins:
        contents = """#!%s
fname = '%s'
with open(fname) as fh:
    exec(compile(fh.read(), fname, 'exec'))
""" % (sys.executable, source_bin)
        dest_bin = os.path.join("bin", os.path.basename(source_bin))
        tools.rewrite(dest_bin, contents, verbose=False)
        os.chmod(dest_bin, 493)  # 493 = 0755, i.e. executable


def link_bin(options, module):
    path = os.path.join("module_bin", options.name)
    tools.mkdir(path, clean=False)
    for old in tools.get_glob([os.path.join(path, "*.py")]):
        os.unlink(old)
    tools.link_dir(os.path.join(module.path, "bin"),
                   path, clean=False, match=["*.py"])


def link_benchmark(options, module):
    path = os.path.join("benchmark", options.name)
    tools.mkdir(path, clean=False)
    for old in tools.get_glob([os.path.join(path, "*.py")]):
        os.unlink(old)
    tools.link_dir(os.path.join(module.path, "benchmark"),
                   path, clean=False, match=["*.py"])


def find_cmdline_links(mod, docdir, cmdline_tools):
    """Look for (sub)sections in the .dox or .md docs for each cmdline tool,
       and return a mapping from tool name to
       (doxygen link, brief desc, num)"""
    links = dict.fromkeys(cmdline_tools)
    num = 0
    todo = {}
    docre = re.compile(r'\\(subsection|section|page)\s+(\S+)\s+(\S+):\s*(.*)$')
    docre_sep = re.compile(r'\\(subsection|section|page)\s+(\S+)\s+(\S+)\s*$')
    mdre = re.compile(r'#*\s*(\S+):\s*([^#]+)#*\s*{#(\S+)}')
    mdre_sep = re.compile(r'#*\s*(\S+)\s*#*\s*{#(\S+)}')
    for g in [os.path.join(docdir, "README.md")] \
            + glob.glob(os.path.join(docdir, "doc", "*.dox")) \
            + glob.glob(os.path.join(docdir, "doc", "*.md")):
        for line in tools.open_utf8(g):
            if todo and len(line.rstrip('\r\n ')) > 0 \
               and line[0] not in " =-\\":
                (k, v) = todo.popitem()
                links[k] = (v, line.rstrip('\r\n '), num)
                num += 1
                todo = {}
            m = docre.search(line)
            if m and m.group(3) in links:
                links[m.group(3)] = (m.group(2), m.group(4), num)
                num += 1
            m = docre_sep.search(line)
            if m and m.group(3) in links:
                todo = {m.group(3): m.group(2)}
            m = mdre.search(line)
            if m and m.group(1) in links:
                links[m.group(1)] = (m.group(3), m.group(2), num)
                num += 1
            m = mdre_sep.search(line)
            if m and m.group(1) in links:
                todo = {m.group(1): m.group(2)}
    missing_links = [tool for tool, link in links.items() if link is None]
    if missing_links:
        print("""
Could not find section title for command line tool %s
in IMP.%s docs.

Each command line tool should have a section or page in the documentation
(in %s/README.md or
%s/doc/*.{dox,md})
that describes it. The section title should contain the tool's name and a
brief description (separated by a colon), followed by a unique doxygen ID.
Alternatively, the brief description can be given in the body immediately
following the title. For example, the tool do_foo.py could be documented with

\\section do_foo_bin do_foo.py: Do something with foo

or

\\section do_foo_bin do_foo.py
Do something with foo

in doxygen (\\subsection or \\page can also be used) or

doo_foo.py: Do something with foo {#do_foo_bin}
=================================

or

# doo_foo.py: Do something with foo {#do_foo_bin}

or

# doo_foo.py {#do_foo_bin}
Do something with foo

in Markdown.
""" % (", ".join(missing_links), mod, docdir, docdir))
        sys.exit(1)
    return links


def make_overview(module, cmdline_tools):
    cmdline_links = find_cmdline_links(module.name, module.path, cmdline_tools)
    pickle.dump(cmdline_links,
                open(os.path.join("build_info",
                                  "IMP_%s.pck" % module.name), 'wb'), -1)
    rmd = tools.open_utf8(os.path.join(module.path, "README.md"), "r").read()
    tools.rewrite(
        os.path.join("doxygen", "generated", "IMP_%s.dox" % module.name),
        """/** \\namespace %s
\\tableofcontents

%s
*/
""" % ('IMP' if module.name == 'kernel' else 'IMP::' + module.name, rmd))


def main():
    options, apps = parser.parse_args()
    with open("build_info/disabled", "r") as fh:
        disabled = tools.split(fh.read(), "\n")
    if options.name in disabled:
        print("%s is disabled" % options.name)
        write_no_ok(options.name)
        tools.rmdir(os.path.join("module_bin", options.name))
        tools.rmdir(os.path.join("benchmark", options.name))
        tools.rmdir(os.path.join("lib", "IMP", options.name))
        sys.exit(1)
    mf = tools.ModulesFinder(source_dir=options.source,
                             external_dir=options.build_dir,
                             module_name=options.name)
    module = mf[options.name]
    success, modules = setup_module(module, mf)
    if success:
        make_header(options, module)
        make_doxygen(options, module, modules, mf)
        make_overview(module, apps)
        link_bin(options, module)
        link_py_apps(module)
        link_benchmark(options, module)
        sys.exit(0)
    else:
        tools.rmdir(os.path.join("module_bin", options.name))
        tools.rmdir(os.path.join("benchmark", options.name))
        tools.rmdir(os.path.join("lib", "IMP", options.name))
        sys.exit(1)


if __name__ == '__main__':
    main()
