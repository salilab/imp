#!/usr/bin/env python

"""
Set up a single application. This involves checking the required modules and
dependencies and then writing data/build_info/IMP.applicationname with whether
the app is ok or not.

Any Python scripts are also linked into the build dir.

If the application cannot be configured, the script exits with an error.
"""

import os.path
import glob
import sys
import copy
import tools
import re
import pickle
from optparse import OptionParser

applist = os.path.join("data", "build_info", "applications")


def write_no_ok(module):
    print "no"
    apps = tools.split(open(applist, "r").read(), "\n")
    apps = [a for a in apps if a != module]
    tools.rewrite(applist, "\n".join(apps))
    tools.rewrite(
        os.path.join(
            "data",
            "build_info",
            "IMP." +
            module),
        "ok=False\n")
    sys.exit(1)


def write_ok(
    module,
    modules,
    unfound_modules,
    dependencies,
        unfound_dependencies):
    print "yes"
    config = ["ok=True"]
    if len(modules) > 0:
        config.append("modules = \"" + ":".join(modules) + "\"")
    if len(unfound_modules) > 0:
        config.append(
            "unfound_modules = \"" +
            ":".join(
                unfound_modules) +
            "\"")
    if len(dependencies) > 0:
        config.append("dependencies = \"" + ":".join(dependencies) + "\"")
    if len(unfound_dependencies) > 0:
        config.append(
            "unfound_dependencies = \"" +
            ":".join(
                unfound_dependencies) +
            "\"")
    apps = tools.split(open(applist, "r").read(), "\n")
    if module not in apps:
        apps.append(module)
    tools.rewrite(applist, "\n".join(apps))
    tools.rewrite(
        os.path.join("data",
                     "build_info",
                     "IMP." + module),
        "\n".join(config))
    sys.exit(0)


def link_py_apps(path):
    tools.mkdir("bin", clean=False)
    tools.link_dir(path, "bin", clean=False, match=["*"],
                   filt=tools.filter_pyapps)


def make_doxygen(name, source, modules):
    file = os.path.join("doxygen", name, "Doxyfile")
    template_file = os.path.join(
        source,
        "tools",
        "build",
        "doxygen_templates",
        "Doxyfile.in")
    template = open(template_file, "r").read()
    template = template.replace("@IMP_SOURCE_PATH@", source)
    template = template.replace("@VERSION@", "0")
    template = template.replace("@NAME@", name)
    template = template.replace("@PROJECT_BRIEF@",
                                '"The Integrative Modeling Platform"')
    template = template.replace("@RECURSIVE@", "YES")
    template = template.replace("@EXCLUDE_PATTERNS@", "*/tutorial/*")
    template = template.replace("@IS_HTML@", "YES")
    template = template.replace("@PROJECT_NAME@", "IMP." + name)
    template = template.replace("@HTML_OUTPUT@", "../../doc/html/" + name)
    template = template.replace("@GENERATE_TAGFILE@", "tags")
    template = template.replace("@IS_XML@", "YES")
    template = template.replace("@XML_OUTPUT@", "xml")
    template = template.replace("@LAYOUT_FILE@",
                                "%s/doc/doxygen/module_layout.xml" % source)
    template = template.replace("@MAINPAGE@", "README.md")
    template = template.replace("@EXAMPLE_PATH@", "")
    template = template.replace("@EXCLUDE@", "")
    template = template.replace("@INCLUDE_PATH@", "")
    template = template.replace("@FILE_PATTERNS@", "*.dox *.md")
    template = template.replace("@WARNINGS@", "warnings.txt")
    # include lib and doxygen in imput
    inputs = []
    docpath = os.path.join(source, "applications", name, "doc")
    if os.path.exists(docpath):
        inputs.append(docpath)
    template = template.replace(
        "@INPUT_PATH@",
        " \\\n                         ".join(inputs))
    tags = []
    for m in modules:
        tags.append(os.path.join("../", m, "tags") + "=" + "../" + m)
    template = template.replace(
        "@TAGS@",
        " \\\n                         ".join(tags))
    tools.rewrite(file, template)

def find_cmdline_links(app, docdir, cmdline_tools):
    """Look for (sub)sections in the .dox or .md docs for each cmdline tool,
       and return a mapping from tool name to (doxygen link, brief desc, num)"""
    links = dict.fromkeys(cmdline_tools)
    num = 0
    todo = {}
    docre = re.compile(r'\\(subsection|section|page)\s+(\S+)\s+(\S+):\s*(.*)$')
    docre_sep = re.compile(r'\\(subsection|section|page)\s+(\S+)\s+(\S+)\s*$')
    mdre = re.compile('#*\s*(\S+):\s*([^#]+)#*\s*{#(\S+)}')
    mdre_sep = re.compile('#*\s*(\S+)\s*#*\s*{#(\S+)}')
    for g in [os.path.join(docdir, "README.md")] \
             + glob.glob(os.path.join(docdir, "doc", "*.dox")) \
             + glob.glob(os.path.join(docdir, "doc", "*.md")):
        for line in open(g):
            if todo and len(line.rstrip('\r\n ')) > 0 \
               and line[0] not in " =-\\":
                k = todo.keys()[0]
                v = todo.values()[0]
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
    missing_links = [tool for tool, link in links.iteritems() if link is None]
    if missing_links:
        print """
Could not find section title for command line tools %s
in %s docs.

Each command line tool should have a section or page in the documentation
(in %s/README.md or
%s/doc/*.{dox,md})
that describes it. The section title should contain the tool's name and a
brief description (separated by a colon), followed by a unique doxygen ID.
Alternatively, the brief description can be given in the body immediately
following the title. For example, the tool do_foo.py could be documented with

\section do_foo_bin do_foo.py: Do something with foo

or

\section do_foo_bin do_foo.py
Do something with foo

in doxygen (\subsection or \page can also be used) or

doo_foo.py: Do something with foo {#do_foo_bin}
=================================

or

# doo_foo.py: Do something with foo {#do_foo_bin}

or

# doo_foo.py {#do_foo_bin}
Do something with foo

in Markdown.
""" % (", ".join(missing_links), app, docdir, docdir)
        sys.exit(1)
    return links

def make_overview(app, source, cmdline_tools):
    docdir = os.path.join(source, "applications", app)
    cmdline_links = find_cmdline_links(app, docdir, cmdline_tools)
    pickle.dump(cmdline_links,
                open(os.path.join("data", "build_info",
                                  "IMP_%s.pck" % app), 'w'), -1)
    rmd = open(os.path.join(docdir, "README.md"), "r").read()
    tools.rewrite(os.path.join("doxygen", "generated", "IMP_%s.dox" % app),
                  """/** \\page imp%s IMP.%s
\\tableofcontents

%s
*/
""" % (app, app, rmd))


def setup_application(application, source, datapath, apps):
    print "Configuring application", application, "...",
    data = tools.get_application_description(source, application, datapath)
    for d in data["required_dependencies"]:
        if not tools.get_dependency_info(d, datapath)["ok"]:
            write_no_ok(application)
            # exits
    dependencies = data["required_dependencies"]
    unfound_dependencies = []
    for d in data["optional_dependencies"]:
        if tools.get_dependency_info(d, datapath)["ok"]:
            dependencies.append(d)
        else:
            unfound_dependencies.append(d)
    for d in data["required_modules"]:
        if not tools.get_module_info(d, datapath)["ok"]:
            write_no_ok(application)
            # exits
    modules = data["required_modules"]
    unfound_modules = []
    for d in data["optional_modules"]:
        if tools.get_module_info(d, datapath)["ok"]:
            modules.append(d)
        else:
            unfound_modules.append(d)
    all_modules = tools.get_dependent_modules(modules, datapath)
    link_py_apps(os.path.join(source, "applications", application))
    make_doxygen(application, source, all_modules)
    make_overview(application, source, apps)
    write_ok(application, all_modules,
             unfound_modules,
             tools.get_dependent_dependencies(
                 all_modules,
                 dependencies,
                 datapath),
             unfound_dependencies)


parser = OptionParser()
parser.add_option("-s", "--source",
                  dest="source", help="IMP source directory.")
parser.add_option("-d", "--datapath",
                  dest="datapath", help="Extra places for IMP data.")
parser.add_option("-n", "--name",
                  dest="name", help="The name of the application.")


def main():
    (options, apps) = parser.parse_args()
    setup_application(options.name, options.source, options.datapath, apps)

if __name__ == '__main__':
    main()
