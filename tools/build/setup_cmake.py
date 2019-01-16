#!/usr/bin/env python

"""
Set up the cmake build scripts for modules. These are written
into the repository directories.
"""

import glob
import os
import sys
import os.path
import shutil
import platform
import tools
import subprocess
from optparse import OptionParser

TOPDIR = os.path.abspath(os.path.dirname(__file__))

check_template = tools.CMakeFileGenerator(os.path.join(TOPDIR,
                                                       "cmake_templates",
                                                       "Check.cmake"))

dep_template = tools.CMakeFileGenerator(os.path.join(TOPDIR,
                                                     "cmake_templates",
                                                     "Dependency.cmake"))

lib_template = tools.CMakeFileGenerator(os.path.join(TOPDIR,
                                                     "cmake_templates",
                                                     "ModuleLib.cmake"))

test_template = tools.CMakeFileGenerator(os.path.join(TOPDIR,
                                                      "cmake_templates",
                                                      "ModuleTest.cmake"))

examples_template = tools.CMakeFileGenerator(os.path.join(TOPDIR,
                                   "cmake_templates", "ModuleExamples.cmake"))

swig_template = tools.CMakeFileGenerator(os.path.join(TOPDIR,
                                                      "cmake_templates",
                                                      "ModuleSwig.cmake"))

util_template = tools.CMakeFileGenerator(os.path.join(TOPDIR,
                                                      "cmake_templates",
                                                      "ModuleUtil.cmake"))

bin_template = tools.CMakeFileGenerator(os.path.join(TOPDIR,
                                                     "cmake_templates",
                                                     "ModuleBin.cmake"))

module_template = tools.CMakeFileGenerator(os.path.join(TOPDIR,
                                             "cmake_templates", "Module.cmake"))

benchmark_template = tools.CMakeFileGenerator(os.path.join(TOPDIR,
                                    "cmake_templates", "ModuleBenchmark.cmake"))

def make_check(path, module, module_path):
    name = os.path.splitext(os.path.split(path)[1])[0]
    cppsource = open(path, "r").read()
    macro = "IMP_COMPILER_%s" % name.upper()
    filename = os.path.join(module_path, "CMakeModules",
                            "Check" + name + ".cmake")
    check_template.write(filename,
                         {"macro": macro,
                          "cppsource": tools.quote(cppsource),
                          "module": module, "name": name})
    defr = "%s=${%s}" % (macro, macro)
    return filename, defr


def make_dependency_check(descr_path, module, module_path):
    descr = tools.get_dependency_description(descr_path)
    name = os.path.splitext(os.path.split(descr_path)[1])[0]
    if len(descr["cmake"]) > 0:
        ret = descr_path[0:-len("description")] + "cmake"
        return ret
    descr["pkgname"] = name
    descr["PKGNAME"] = name.upper()
    filename = os.path.join(
        module_path,
        "CMakeModules",
        "Find" + name + ".cmake")
    if descr["python_module"] != "":
        # don't bother checking python deps as they aren't needed for compilation
        # and it makes cross compilation easier
        return None
    else:
        descr["pkgconfigs"] = ";".join(descr["pkg_config_name"])
        descr["includes"] = "\n".join(["#include <%s>" % h
                                       for h in descr["headers"]])
        descr["headers"] = ";".join(descr["headers"])
        descr["libraries"] = ";".join(descr["libraries"])
        descr["body"] = tools.quote(descr["body"])
        if len(descr["cmake"]) > 0:
            descr["path"] = os.path.splitext(descr_path)[0]
            descr["on_failure"] = """set(%(PKGNAME)s_INTERNAL 1 CACHE INTERNAL "" FORCE)
        %(cmake)s
""" % descr
            descr["on_setup"] = """if(DEFINED %(PKGNAME)s_INTERNAL)
%(cmake)s
endif(DEFINED %(PKGNAME)s_INTERNAL)""" % descr
        else:
            descr["on_failure"] = """message("%s not found")\nfile(WRITE "${CMAKE_BINARY_DIR}/data/build_info/%s" "ok=False")""" % (
                descr['full_name'], name)
            descr["on_setup"] = ""
        dep_template.write(filename, descr)
    return filename


def get_sources(module, path, subdir, pattern):
    matching = tools.get_glob([os.path.join(path, subdir, pattern),
                              os.path.join(path, subdir, "*", pattern)])
    return "\n".join(["${CMAKE_SOURCE_DIR}/%s"
                     % tools.to_cmake_path(x) for x in matching])

def get_app_sources(path, patterns, filt=lambda x:True):
    matching = tools.get_glob([os.path.join(path, x) for x in patterns])
    return ["${CMAKE_SOURCE_DIR}/%s" % tools.to_cmake_path(x) \
            for x in matching if filt(x)]

def get_dep_merged(modules, name, ordered):
    ret = []
    alldeps = tools.get_all_dependencies(".", modules, "", ordered)
    for d in alldeps:
        ret.append("${%s_%s}" % (d.upper(), name.upper()))
    ret = sorted(set(ret))
    return "\n        ".join(ret)


def setup_module(module, path, ordered):
    checks = []
    deps = []
    contents = []
    defines = []
    for cc in tools.get_glob([os.path.join(path, "compiler", "*.cpp")]):
        ret = make_check(cc, module, path)
        checks.append(ret[0])
        defines.append(ret[1])
    for cc in tools.get_glob([os.path.join(path, "dependency", "*.description")]):
        ret = make_dependency_check(cc, module, path)
        if ret:
            deps.append(ret)

    g = tools.CMakeFileGenerator()
    if len(checks) > 0:
        g.write("modules/%s/compiler/CMakeLists.txt" % module,
                "\n".join(["include(${CMAKE_SOURCE_DIR}/%s)\n" %
                               tools.to_cmake_path(x) for x in checks]))
        contents.append(
            "add_subdirectory(${CMAKE_SOURCE_DIR}/modules/%s/compiler)" %
            module)
    if len(deps) > 0:
        g.write("modules/%s/dependency/CMakeLists.txt" % module,
                "\n".join(["include(${CMAKE_SOURCE_DIR}/%s)" %
                               tools.to_cmake_path(x) for x in deps]))
        contents.append(
            "add_subdirectory(${CMAKE_SOURCE_DIR}/modules/%s/dependency)" %
            module)
    local = os.path.join(path, "Setup.cmake")
    if os.path.exists(local):
        contents.append("include(${CMAKE_SOURCE_DIR}/%s)"
                        % tools.to_cmake_path(local))

    values = {"name": module}
    if module == 'kernel':
        values['subdir'] = 'IMP'
        values['pymod'] = 'IMP'
        values['allh_header'] = 'IMP.h'
    else:
        values['subdir'] = 'IMP/' + module
        values['pymod'] = 'IMP.' + module
        values['allh_header'] = 'IMP/%s.h' % module
    values["NAME"] = module.upper()
    values["CPPNAME"] = module.upper().replace('_', '')
    data = tools.get_module_description(".", module, "")
    all_modules = tools.get_all_modules(".", [module], "", ordered)
    modules = ["${IMP_%s_LIBRARY}" % s for s in all_modules]
    dependencies = ["${%s_LIBRARIES}" % s.upper()
                    for s in tools.get_all_dependencies(".", [module], "", ordered)]
    values["modules"] = ";".join(modules)
    values["tags"] = "\n".join(["${IMP_%s_DOC}" % m for m in all_modules])
    values["other_pythons"] = "\n".join(
        ["${IMP_%s_PYTHON}" %
         m for m in all_modules])
    values["dependencies"] = ";".join(dependencies)
    values["headers"] = get_sources(module, path, "include", "*.h")
    values["includepath"] = get_dep_merged([module], "include_path", ordered)
    values["libpath"] = get_dep_merged([module], "link_path", ordered)
    values["swigpath"] = get_dep_merged([module], "swig_path", ordered)
    values["defines"] = ":".join(defines)
    cppbins = tools.get_glob([os.path.join(path, "bin", "*.cpp")])
    cppbins = [os.path.splitext(e)[0] for e in cppbins]
    pybins = get_app_sources(os.path.join(path, "bin"), ["*"],
                             tools.filter_pyapps)
    values["pybins"] = "\n".join(pybins)
    values["bin_names"] = "\n".join([os.path.basename(x) \
                                     for x in pybins + cppbins])

    local = os.path.join(path, "Build.cmake")
    if os.path.exists(local):
        values["custom_build"] = "include(${CMAKE_SOURCE_DIR}/%s)\n" \
                                 % tools.to_cmake_path(local)
    else:
        values["custom_build"] = ""

    main = os.path.join(path, "src", "CMakeLists.txt")
    tests = os.path.join(path, "test", "CMakeLists.txt")
    swig = os.path.join(path, "pyext", "CMakeLists.txt")
    util = os.path.join(path, "utility", "CMakeLists.txt")
    bin = os.path.join(path, "bin", "CMakeLists.txt")
    benchmark = os.path.join(path, "benchmark", "CMakeLists.txt")
    examples = os.path.join(path, "examples", "CMakeLists.txt")
    lib_template.write(main, values)
    test_template.write(tests, values)
    swig_template.write(swig, values)
    util_template.write(util, values)
    bin_template.write(bin, values)
    benchmark_template.write(benchmark, values)
    examples_template.write(examples, values)
    values["tests"] = "\n".join(contents)
    values["subdirs"] = """add_subdirectory(${CMAKE_SOURCE_DIR}/modules/%s/src)
add_subdirectory(${CMAKE_SOURCE_DIR}/modules/%s/test)
add_subdirectory(${CMAKE_SOURCE_DIR}/modules/%s/examples)
add_subdirectory(${CMAKE_SOURCE_DIR}/modules/%s/benchmark)
add_subdirectory(${CMAKE_SOURCE_DIR}/modules/%s/bin)
add_subdirectory(${CMAKE_SOURCE_DIR}/modules/%s/utility)""" % ((module,) * 6)

    out = os.path.join(path, "CMakeLists.txt")
    module_template.write(out, values)

    # at end so directories exist
    subprocess.check_call(["python",
                          os.path.join(TOPDIR, "..",
                                       "dev_tools", "setup_cmake.py")],
                          cwd=path, universal_newlines=True)

    return out


parser = OptionParser()


def main():
    (options, args) = parser.parse_args()
    main = []
    ordered = tools.compute_sorted_order(".", "")
    for m in ordered:
        """if m not in ["base", "kernel", "algebra", "cgal", "test", "statistics", "display", "core", "kmeans", "score_functor",
           "container", "atom", "rmf", "domino", "example"]:
            continue"""
        p = os.path.join("modules", m)
        main.append(setup_module(m, p, ordered))

if __name__ == '__main__':
    main()
