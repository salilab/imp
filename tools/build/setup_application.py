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
from optparse import OptionParser

applist=os.path.join("data", "build_info", "applications")

def write_no_ok(module):
    print "no"
    apps= tools.split(open(applist, "r").read(), "\n")
    apps= [a for a in apps if a != module]
    tools.rewrite(applist, "\n".join(apps))
    tools.rewrite(os.path.join("data", "build_info", "IMP."+module), "ok=False\n")
    sys.exit(1)

def write_ok(module, modules, unfound_modules, dependencies, unfound_dependencies):
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
    apps= tools.split(open(applist, "r").read(), "\n")
    if module not in apps:
        apps.append(module)
    tools.rewrite(applist, "\n".join(apps))
    tools.rewrite(os.path.join("data", "build_info", "IMP."+module), "\n".join(config))
    sys.exit(0)

def link_py(path):
    tools.mkdir("bin", clean=False)
    tools.link_dir(path, "bin", clean=False, match=["*.py"],
                   exclude=['dependencies.py'])


def make_doxygen(name, source, modules):
    file = os.path.join("doxygen", name, "Doxyfile")
    template_file = os.path.join(source, "tools", "build", "doxygen_templates", "Doxyfile.in")
    template = open(template_file, "r").read()
    template = template.replace("@IMP_SOURCE_PATH@", source)
    template = template.replace("@VERSION@", "0")
    template = template.replace("@NAME@", name)
    template = template.replace("@PROJECT_BRIEF@",
                                '"The Integrative Modeling Platform"')
    template = template.replace("@RECURSIVE@", "YES")
    template = template.replace("@EXCLUDE_PATTERNS@", "*/tutorial/*")
    template = template.replace("@IS_HTML@", "YES")
    template = template.replace("@PROJECT_NAME@", "IMP."+name)
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
    inputs.append(source + "/applications/" + name + "/doc")
    template = template.replace("@INPUT_PATH@", " \\\n                         ".join(inputs))
    tags = []
    for m in modules:
        tags.append(os.path.join("../", m, "tags") + "=" + "../"+m)
    template = template.replace("@TAGS@", " \\\n                         ".join(tags))
    tools.rewrite(file, template)

def make_overview(app, source):
    rmd = open(os.path.join(source, "applications", app, "README.md"), "r").read()
    tools.rewrite(os.path.join("doxygen", "generated", "IMP_%s.dox" % app),
                  """/** \\page imp%s IMP.%s
\\tableofcontents

%s
*/
""" %(app, app, rmd))


def setup_application(application, source, datapath):
    print "Configuring application", application, "...",
    data= tools.get_application_description(source, application, datapath)
    for d in data["required_dependencies"]:
        if not tools.get_dependency_info(d, datapath)["ok"]:
            write_no_ok(application)
            #exits
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
    modules= data["required_modules"]
    unfound_modules = []
    for d in data["optional_modules"]:
        if tools.get_module_info(d, datapath)["ok"]:
            modules.append(d)
        else:
            unfound_modules.append(d)
    all_modules=tools.get_dependent_modules(modules, datapath)
    link_py(os.path.join(source, "applications", application))
    make_doxygen(application, source, all_modules)
    make_overview(application, source)
    write_ok(application, all_modules,
             unfound_modules,
        tools.get_dependent_dependencies(all_modules, dependencies, datapath),
        unfound_dependencies)


parser = OptionParser()
parser.add_option("-s", "--source",
                  dest="source", help="IMP source directory.")
parser.add_option("-d", "--datapath",
                  dest="datapath", help="Extra places for IMP data.")
parser.add_option("-n", "--name",
                  dest="name", help="The name of the application.")

def main():
    (options, args) = parser.parse_args()
    setup_application(options.name, options.source, options.datapath)

if __name__ == '__main__':
    main()
