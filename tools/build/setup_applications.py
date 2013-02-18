#!/usr/bin/python
""" Should be replaced by setup_application.py
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

def link_py(path):
    tools.mkdir("bin", clean=False)
    tools.link_dir(path, "bin", clean=False, match=["*.py"])


def setup_application(application, source, datapath):
    print "Configuring application", application, "...",
    data= tools.get_application_description(source, application, datapath)
    for d in data["required_dependencies"]:
        if not tools.get_dependency_info(d, datapath)["ok"]:
            write_no_ok(application)
            return
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
            return
    modules= data["required_modules"]
    unfound_modules = []
    for d in data["optional_modules"]:
        if tools.get_module_info(d, datapath)["ok"]:
            modules.append(d)
        else:
            unfound_modules.append(d)
    all_modules=tools.get_dependent_modules(modules, datapath)
    write_ok(application, all_modules,
             unfound_modules,
        tools.get_dependent_dependencies(all_modules, dependencies, datapath),
        unfound_dependencies)
    link_py(os.path.join("applications", application))


parser = OptionParser()
parser.add_option("-s", "--source",
                  dest="source", help="IMP source directory.")
parser.add_option("-d", "--datapath",
                  dest="datapath", help="Extra places for IMP data.")

def main():
    (options, args) = parser.parse_args()
    for m, path in tools.get_applications(options.source):
        setup_application(m, options.source, options.datapath)

if __name__ == '__main__':
    main()
