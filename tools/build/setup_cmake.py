#!/usr/bin/env python

"""
Set up the cmake build scripts for modules and applications. These are written
into the repository directories.
"""

import glob
import os
import sys
import os.path
import shutil
import platform
import tools
from optparse import OptionParser

check_template = open(os.path.join("tools", "build", "cmake_templates", "Check.cmake"), "r").read()

dep_template = open(os.path.join("tools", "build", "cmake_templates", "Dependency.cmake"), "r").read()

lib_template = open(os.path.join("tools", "build", "cmake_templates", "ModuleLib.cmake"), "r").read()

test_template = open(os.path.join("tools", "build", "cmake_templates", "ModuleTest.cmake"), "r").read()

examples_template = open(os.path.join("tools", "build", "cmake_templates", "ModuleExamples.cmake"), "r").read()

swig_template = open(os.path.join("tools", "build", "cmake_templates", "ModuleSwig.cmake"), "r").read()

bin_template = open(os.path.join("tools", "build", "cmake_templates", "ModuleBin.cmake"), "r").read()

module_template = open(os.path.join("tools", "build", "cmake_templates", "Module.cmake"), "r").read()

benchmark_template = open(os.path.join("tools", "build", "cmake_templates", "ModuleBenchmark.cmake"), "r").read()

application_template = open(os.path.join("tools", "build", "cmake_templates", "Application.cmake"), "r").read()

def make_check(path, module, module_path):
    name= os.path.splitext(os.path.split(path)[1])[0]
    cppsource= open(path, "r").read()
    macro="IMP_COMPILER_%s"%name.upper()
    output=check_template%{"macro":macro, "cppsource":tools.quote(cppsource), "module":module, "name":name}
    filename=os.path.join(module_path, "CMakeModules", "Check"+name+".cmake")
    tools.rewrite(filename, output)
    defr="%s=${%s}"%(macro, macro)
    return filename, defr

def make_dependency_check(descr_path, module, module_path):
    descr= tools.get_dependency_description(descr_path)
    name= os.path.splitext(os.path.split(descr_path)[1])[0]
    descr["pkgname"]=name
    descr["PKGNAME"]=name.upper()
    filename=os.path.join(module_path, "CMakeModules", "Find"+name+".cmake")
    if descr["python_module"] != "":
        # don't bother checking python deps as they aren't needed for compilation
        # and it makes cross compilation easier
        return None
    else:
        descr["includes"]= "\n".join(["#include <%s>" % h \
                                     for h in descr["headers"]])
        descr["headers"]= "\n".join(descr["headers"])
        descr["libraries"]= "\n".join(descr["libraries"])
        descr["body"]= tools.quote(descr["body"])
        if len(descr["cmake"])>0:
            descr["path"]=os.path.splitext(descr_path)[0]
            descr["on_failure"]="""set(%(PKGNAME)s_INTERNAL 1 CACHE INTERNAL "" FORCE)
        %(cmake)s
"""%descr
            descr["on_setup"]= """if(DEFINED %(PKGNAME)s_INTERNAL)
%(cmake)s
endif(DEFINED %(PKGNAME)s_INTERNAL)"""%descr
        else:
            descr["on_failure"]="""message("%s not found")\nfile(WRITE "${PROJECT_BINARY_DIR}/data/build_info/%s" "ok=False")"""%(name, name)
            descr["on_setup"]=""
        output=dep_template%descr
        tools.rewrite(filename, output)
    return filename

def get_sources(module, path, subdir, pattern):
    matching = tools.get_glob([os.path.join(path, subdir, pattern),
                              os.path.join(path, subdir, "*", pattern)])
    return "\n".join(["${PROJECT_SOURCE_DIR}/%s" \
                     % tools.to_cmake_path(x) for x in matching])

def get_app_sources(path, patterns):
    matching = tools.get_glob([os.path.join(path, x) for x in patterns])
    return "\n".join(["${PROJECT_SOURCE_DIR}/%s" \
                     % tools.to_cmake_path(x) for x in matching])

def get_dep_merged(modules, name, ordered):
    ret=[]
    alldeps=tools.get_all_dependencies(".", modules, "", ordered)
    for d in alldeps:
        ret.append("${%s_%s}"%(d.upper(), name.upper()))
    ret=list(set(ret))
    ret.sort()
    return "\n        ".join(ret)

def setup_module(module, path, ordered):
    checks=[]
    deps=[]
    contents=[]
    defines=[]
    for cc in tools.get_glob([os.path.join(path, "compiler", "*.cpp")]):
        ret=make_check(cc, module, path)
        checks.append(ret[0])
        defines.append(ret[1])
    for cc in tools.get_glob([os.path.join(path, "dependency", "*.description")]):
        ret=make_dependency_check(cc, module, path)
        if ret:
            deps.append(ret)

    if len(checks) > 0:
        tools.rewrite("modules/%s/compiler/CMakeLists.txt"%module, "\n".join(["include(${PROJECT_SOURCE_DIR}/%s)\n"%tools.to_cmake_path(x) for x in checks]))
        contents.append("add_subdirectory(${PROJECT_SOURCE_DIR}/modules/%s/compiler)"%module)
    if len(deps) > 0:
        tools.rewrite("modules/%s/dependency/CMakeLists.txt"%module, "\n".join(["include(${PROJECT_SOURCE_DIR}/%s)"%tools.to_cmake_path(x) for x in deps]))
        contents.append("add_subdirectory(${PROJECT_SOURCE_DIR}/modules/%s/dependency)"%module)
    local=os.path.join(path, "Setup.cmake")
    if os.path.exists(local):
        contents.append("include(${PROJECT_SOURCE_DIR}/%s)" \
                        % tools.to_cmake_path(local))

    values= {"name":module}
    values["NAME"]=module.upper()
    values["CPPNAME"]=module.upper().replace('_', '')
    data=tools.get_module_description(".", module, "")
    modules=["${IMP_%s_LIBRARY}"%s.upper() for s in tools.get_all_modules(".", [module], "", ordered)]
    dependencies=["${%s_LIBRARIES}"%s.upper() for s in tools.get_all_dependencies(".", [module], "", ordered)]
    values["modules"]="\n".join(modules)
    values["dependencies"]="\n".join(dependencies)
    values["sources"] = get_sources(module, path, "src", "*.cpp")
    values["headers"] = get_sources(module, path, "include", "*.h")
    values["cppbins"] = get_sources(module, path, "bin", "*.cpp")
    values["cppbenchmarks"] = get_sources(module, path, "benchmark", "*.cpp")
    values["pybenchmarks"] = get_sources(module, path, "benchmark", "*.py")
    values["pytests"] = get_sources(module, path, "test", "test_*.py")
    values["expytests"] = get_sources(module, path, "test", "expensive_test_*.py")
    values["mdpytests"] = get_sources(module, path, "test", "medium_test_*.py")
    values["cpptests"] = get_sources(module, path, "test", "test_*.cpp")
    values["pyexamples"] = get_sources(module, path, "examples", "*.py")
    values["cppexamples"] = get_sources(module, path, "examples", "*.cpp")
    values["excpptests"] = get_sources(module, path, "test", "expensive_test_*.cpp")
    values["includepath"] = get_dep_merged([module], "include_path", ordered)
    values["libpath"] = get_dep_merged([module], "link_path", ordered)
    values["swigpath"] = get_dep_merged([module], "swig_path", ordered)
    values["defines"] = ":".join(defines)

    main= os.path.join(path, "src", "CMakeLists.txt")
    tests= os.path.join(path, "test", "CMakeLists.txt")
    swig= os.path.join(path, "pyext", "CMakeLists.txt")
    bin= os.path.join(path, "bin", "CMakeLists.txt")
    benchmark= os.path.join(path, "benchmark", "CMakeLists.txt")
    examples= os.path.join(path, "examples", "CMakeLists.txt")
    tools.rewrite(main, lib_template%values)
    tools.rewrite(tests, test_template%values)
    tools.rewrite(swig, swig_template%values)
    tools.rewrite(bin, bin_template%values)
    tools.rewrite(benchmark, benchmark_template%values)
    tools.rewrite(examples, examples_template%values)
    values["tests"] = "\n".join(contents)
    values["subdirs"] = """add_subdirectory(${PROJECT_SOURCE_DIR}/modules/%s/src)
add_subdirectory(${PROJECT_SOURCE_DIR}/modules/%s/test)
add_subdirectory(${PROJECT_SOURCE_DIR}/modules/%s/examples)
add_subdirectory(${PROJECT_SOURCE_DIR}/modules/%s/benchmark)
add_subdirectory(${PROJECT_SOURCE_DIR}/modules/%s/bin)"""%((module,)*5)

    out=os.path.join(path, "CMakeLists.txt")
    tools.rewrite(out, module_template%values)
    return out

def setup_application(options, name, ordered):
    contents=[]
    path= os.path.join("applications", name)
    local=os.path.join(path, "Setup.cmake")
    if os.path.exists(local):
        contents.append("include(%s)" % tools.to_cmake_path(local))

    values= {"name":name}
    values["NAME"]=name.upper()
    data=tools.get_application_description(".", name, "")
    all_modules=data["required_modules"]+tools.get_all_modules(".", data["required_modules"], "", ordered)
    all_dependencies=tools.get_all_dependencies(".", all_modules, "", ordered)
    modules=["${IMP_%s_LIBRARY}"%s.upper() for s in all_modules]
    dependencies=["${%s_LIBRARIES}"%s.upper() for s in all_dependencies]
    values["modules"]="\n".join(modules)
    values["dependencies"]="\n".join(dependencies)
    exes= tools.get_application_executables(path)
    exedirs = list(set(sum([x[1] for x in exes], [])))
    exedirs.sort()
    localincludes="\n     ".join(["${PROJECT_SOURCE_DIR}/"+tools.to_cmake_path(x) for x in exedirs])
    bintmpl="""
   add_executable("%(cname)s" %(cpps)s)
   target_link_libraries(%(cname)s
    %(modules)s
    %(dependencies)s)
   set_target_properties(%(cname)s PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
   install(TARGETS %(cname)s DESTINATION ${CMAKE_INSTALL_BINDIR})
   set(bins ${bins} %(cname)s)
"""
    bins=[]
    for e in exes:
        cpps= e[0]
        cname= os.path.splitext(os.path.split(cpps[0])[1])[0]
        values["cname"]=cname
        values["cpps"]= "\n".join(["${PROJECT_SOURCE_DIR}/%s" \
                                  % tools.to_cmake_path(c) for c in cpps])
        bins.append(bintmpl%values)
    values["bins"] = "\n".join(bins) + """
add_custom_target("IMP.%s" ALL DEPENDS ${bins})
""" % values["name"]
    values["includepath"] = get_dep_merged(all_modules, "include_path", ordered)+" "+localincludes
    values["libpath"] = get_dep_merged(all_modules, "link_path", ordered)
    values["swigpath"] = get_dep_merged(all_modules, "swig_path", ordered)
    values["pybins"] = get_app_sources(path, ["*.py"])
    values["pytests"] = get_app_sources(os.path.join(path, "test"),
                                        ["test_*.py",
                                         "expensive_test_*.py"])
    contents.append(application_template%values)
    out=os.path.join(path, "CMakeLists.txt")
    tools.rewrite(out, "\n".join(contents))
    return out

parser = OptionParser()

def main():
    (options, args) = parser.parse_args()
    main=[]
    ordered=tools.compute_sorted_order(".", "")
    for m in ordered:
        """if m not in ["base", "kernel", "algebra", "cgal", "test", "statistics", "display", "core", "kmeans", "score_functor",
           "container", "atom", "rmf", "domino", "example"]:
            continue"""
        p = os.path.join("modules", m)
        main.append(setup_module(m, p, ordered))


    for a in [x for x in tools.get_glob([os.path.join("applications", "*")]) if os.path.isdir(x)]:
        main.append(setup_application(options, os.path.split(a)[1], ordered))
    #contents=["include(${PROJECT_SOURCE_DIR}/%s)"%x for x in main]
    #tools.rewrite(os.path.join("cmake", "CMakeLists.txt"), "\n".join(contents))

if __name__ == '__main__':
    main()
