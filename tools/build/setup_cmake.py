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

def make_check(path, module):
    name = os.path.splitext(os.path.split(path)[1])[0]
    cppsource = open(path, "r").read()
    macro = "IMP_COMPILER_%s" % name.upper()
    filename = os.path.join(module.path, "CMakeModules",
                            "Check" + name + ".cmake")
    check_template.write(filename,
                         {"macro": macro,
                          "cppsource": tools.quote(cppsource),
                          "module": module.name, "name": name})
    defr = "%s=${%s}" % (macro, macro)
    return filename, defr


def make_dependency_check(descr_path, module):
    descr = tools.get_dependency_description(descr_path)
    name = os.path.splitext(os.path.split(descr_path)[1])[0]
    if len(descr["cmake"]) > 0:
        ret = descr_path[0:-len("description")] + "cmake"
        return ret
    descr["pkgname"] = name
    descr["PKGNAME"] = name.upper()
    filename = os.path.join(
        module.path,
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
            descr["on_failure"] = """message("%s not found")\nfile(WRITE "${CMAKE_BINARY_DIR}/build_info/%s" "ok=False")""" % (
                descr['full_name'], name)
            descr["on_setup"] = ""
        dep_template.write(filename, descr)
    return filename


def get_sources(module, subdir, pattern):
    matching = tools.get_glob([os.path.join(module.path, subdir, pattern),
                               os.path.join(module.path, subdir, "*", pattern)])
    return "\n".join(["${CMAKE_SOURCE_DIR}/%s"
                     % tools.to_cmake_path(x) for x in matching])

def get_app_sources(path, patterns, filt=lambda x:True):
    matching = tools.get_glob([os.path.join(path, x) for x in patterns])
    return ["${CMAKE_SOURCE_DIR}/%s" % tools.to_cmake_path(x) \
            for x in matching if filt(x)]

def get_dep_merged(all_deps, name):
    return "\n        ".join(sorted(set("${%s_%s}" % (d.upper(), name.upper())
                                    for d in all_deps)))

def standalone_cmake(cmake_file):
    """Return True iff the given cmake file is set up to work standalone.
       Such a file can have cmake run directly on it (usually used for
       out-of-tree module builds).
       If the file is standalone capable, we won't overwrite it but instead
       will generate a ModuleBuild.cmake file. The cmake file is supposed to
       include ModuleBuild.cmake if it detects it is not running standalone.
    """
    if os.path.exists(cmake_file):
        with open(cmake_file) as fh:
            return 'ModuleBuild.cmake' in fh.read()

def setup_module(finder, module, tools_dir, extra_include, extra_swig,
                 required):
    checks = []
    deps = []
    contents = []
    defines = []
    for cc in tools.get_glob([os.path.join(module.path, "compiler", "*.cpp")]):
        ret = make_check(cc, module)
        checks.append(ret[0])
        defines.append(ret[1])
    for cc in tools.get_glob([os.path.join(module.path, "dependency",
                                           "*.description")]):
        ret = make_dependency_check(cc, module)
        if ret:
            deps.append(ret)

    g = tools.CMakeFileGenerator()
    if len(checks) > 0:
        g.write("%s/compiler/CMakeLists.txt" % module.path,
                "\n".join(["include(${CMAKE_SOURCE_DIR}/%s)\n" %
                               tools.to_cmake_path(x) for x in checks]))
        contents.append(
            "add_subdirectory(${CMAKE_SOURCE_DIR}/%s/compiler)" %
            module.path)
    if len(deps) > 0:
        g.write("%s/dependency/CMakeLists.txt" % module.path,
                "\n".join(["include(${CMAKE_SOURCE_DIR}/%s)" %
                               tools.to_cmake_path(x) for x in deps]))
        contents.append(
            "add_subdirectory(${CMAKE_SOURCE_DIR}/%s/dependency)" %
            module.path)
    local = os.path.join(module.path, "Setup.cmake")
    if os.path.exists(local):
        contents.append("include(${CMAKE_SOURCE_DIR}/%s)"
                        % tools.to_cmake_path(local))

    values = {"name": module.name, "extra_include": extra_include,
              "extra_swig": extra_swig,
              "module_dir": tools.to_cmake_path(module.path) + '/'
                            if module.path else '',
              "tools_dir": tools.to_cmake_path(tools_dir) + '/'
                           if tools_dir else ''}
    if module.name == 'kernel':
        values['subdir'] = 'IMP'
        values['pymod'] = 'IMP'
        values['allh_header'] = 'IMP.h'
    else:
        values['subdir'] = 'IMP/' + module.name
        values['pymod'] = 'IMP.' + module.name
        values['allh_header'] = 'IMP/%s.h' % module.name
    values["NAME"] = module.name.upper()
    values["CPPNAME"] = module.name.upper().replace('_', '')
    all_modules = module.get_all_modules()
    modules = ["${IMP_%s_LIBRARY}" % s.name for s in all_modules]
    all_dependencies = list(finder.get_all_dependencies([module] + all_modules))
    dependencies = ["${%s_LIBRARIES}" % s.upper() for s in all_dependencies]
    values["modules"] = ";".join(modules)
    values["tags"] = "\n".join(["${IMP_%s_DOC}" % m.name for m in all_modules])
    values["other_pythons"] = "\n".join(
        ["${IMP_%s_PYTHON}" %
         m.name for m in all_modules])
    values["dependencies"] = ";".join(dependencies)
    values["headers"] = get_sources(module, "include", "*.h")
    values["includepath"] = get_dep_merged(all_dependencies, "include_path")
    values["libpath"] = get_dep_merged(all_dependencies, "link_path")
    values["swigpath"] = get_dep_merged(all_dependencies, "swig_path")
    values["defines"] = ":".join(defines)
    cppbins = tools.get_glob([os.path.join(module.path, "bin", "*.cpp")])
    cppbins = [os.path.splitext(e)[0] for e in cppbins]
    pybins = get_app_sources(os.path.join(module.path, "bin"), ["*"],
                             tools.filter_pyapps)
    values["pybins"] = "\n".join(pybins)
    values["bin_names"] = "\n".join([os.path.basename(x) \
                                     for x in pybins + cppbins])

    local = os.path.join(module.path, "Build.cmake")
    if os.path.exists(local):
        values["custom_build"] = "include(${CMAKE_SOURCE_DIR}/%s)\n" \
                                 % tools.to_cmake_path(local)
    else:
        values["custom_build"] = ""

    main = os.path.join(module.path, "src", "CMakeLists.txt")
    tests = os.path.join(module.path, "test", "CMakeLists.txt")
    swig = os.path.join(module.path, "pyext", "CMakeLists.txt")
    util = os.path.join(module.path, "utility", "CMakeLists.txt")
    bin = os.path.join(module.path, "bin", "CMakeLists.txt")
    benchmark = os.path.join(module.path, "benchmark", "CMakeLists.txt")
    examples = os.path.join(module.path, "examples", "CMakeLists.txt")
    lib_template.write(main, values)
    test_template.write(tests, values)
    swig_template.write(swig, values)
    util_template.write(util, values)
    bin_template.write(bin, values)
    benchmark_template.write(benchmark, values)
    examples_template.write(examples, values)
    values["tests"] = "\n".join(contents)
    topdir = '/' + module.path if module.path else ''
    if finder.external_dir:
        values["build_dir"] = "--build_dir=%s " % finder.external_dir
    else:
        values["build_dir"] = ""
    values["disabled_status"] = "FATAL_ERROR" if required else "STATUS"
    values["subdirs"] = """add_subdirectory(${CMAKE_SOURCE_DIR}%s/src)
add_subdirectory(${CMAKE_SOURCE_DIR}%s/test)
add_subdirectory(${CMAKE_SOURCE_DIR}%s/examples)
add_subdirectory(${CMAKE_SOURCE_DIR}%s/benchmark)
add_subdirectory(${CMAKE_SOURCE_DIR}%s/bin)
add_subdirectory(${CMAKE_SOURCE_DIR}%s/utility)""" % ((topdir,) * 6)

    cmakelists = os.path.join(module.path, "CMakeLists.txt")
    if finder.one_module or standalone_cmake(cmakelists):
        out = os.path.join(module.path, "ModuleBuild.cmake")
    else:
        out = cmakelists
    module_template.write(out, values)

    # at end so directories exist
    subprocess.check_call(["python",
                          os.path.join(TOPDIR, "..",
                                       "dev_tools", "setup_cmake.py")],
                          cwd=module.path or '.', universal_newlines=True)

    return out


parser = OptionParser()
parser.add_option("--include", help="Extra header include path", default=None)
parser.add_option("--swig_include", help="Extra SWIG include path",
                  default=None)
parser.add_option("--build_dir", help="IMP build directory", default=None)
parser.add_option("--tools_dir", help="IMP tools directory", default=None)
parser.add_option("--required", action="store_true", default=False,
                  help="Whether to fail the build if a module cannot "
                       "be configured")


def main():
    (options, args) = parser.parse_args()
    main = []
    mf = tools.ModulesFinder(source_dir='', external_dir=options.build_dir)
    tools_dir = options.tools_dir if options.tools_dir \
                                  else '${CMAKE_SOURCE_DIR}/tools'
    extra_include = ' "--include=%s"' % options.include \
                    if options.include else ""
    extra_swig = ' "--swig_include=%s"' % options.swig_include \
                    if options.swig_include else ""
    for m in mf.get_ordered():
        if isinstance(m, tools.SourceModule):
            main.append(setup_module(mf, m, tools_dir, extra_include,
                                     extra_swig, options.required))

if __name__ == '__main__':
    main()
