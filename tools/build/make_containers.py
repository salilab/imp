#!/usr/bin/env python

"""
Generate the headers and source files for the container related types from the
templates in tools/build/container_templates. These are written to the
build directory.
"""

import sys
import os
import os.path
import tools

all_inputs = []
all_outputs = []


def filter(xxx_todo_changeme,
           infile, file_name):
    (function_name, type_name, class_name, variable_type, argument_type,
     return_type, storage_type, plural_variable_type, plural_argument_type,
     plural_storage_type, index_type, plural_index_type,
     pass_index_type) = xxx_todo_changeme
    helpername = class_name.lower()
    return infile\
        .replace("HELPERNAME", helpername)\
        .replace("CLASSFUNCTIONNAME", helpername)\
        .replace("FUNCTIONNAME", function_name)\
        .replace("TYPENAME", type_name)\
        .replace("classname", class_name.lower())\
        .replace("Classname", class_name)\
        .replace("CLASSNAME", class_name.upper())\
        .replace("PLURALVARIABLETYPE", plural_variable_type)\
        .replace("PLURALARGUMENTTYPE", plural_argument_type)\
        .replace("PLURALINDEXTYPE", plural_index_type)\
        .replace("PASSINDEXTYPE", pass_index_type)\
        .replace("INDEXTYPE", index_type)\
        .replace("RETURNTYPE", return_type)\
        .replace("VARIABLETYPE", variable_type)\
        .replace("PLURALSTORAGETYPE", plural_storage_type)\
        .replace("ARGUMENTTYPE", argument_type)\
        .replace("STORAGETYPE", storage_type)\
        .replace("FILESOURCE", file_name)


class ContainerFileGenerator(tools.FileGenerator):
    def __init__(self, template_file):
        if template_file.endswith('.py'):
            tools.FileGenerator.__init__(self, template_file, '#')
        else:
            tools.FileGenerator.__init__(self, template_file, '//')

    def get_output_file_contents(self, output):
        return filter(output, self.template, self.template_file)


def make_one(path, params, test=True):
    (function_name, type_name, class_name, variable_type, argument_type,
     return_type, storage_type, plural_variable_type, plural_argument_type,
     plural_storage_type, index_type, plural_index_type,
     pass_index_type) = params
    cname = function_name

    inputs = tools.get_glob([os.path.join(path, "*", "*.h"),
                            os.path.join(path, "*", "internal", "*.h"),
                            os.path.join(path, "*", "*.cpp"),
                            os.path.join(path, "*", "internal", "*.cpp")])
    files = []
    for ip in inputs:
        p = ip[len(path) + 1:]
        module = os.path.split(p)[0]
        rest = os.path.split(p)[1]
        if module.find("internal") != -1:
            module = os.path.split(module)[0]
            rest = os.path.join("internal", rest)
        name = filter(params, rest, rest)
        if p.endswith(".h"):
            out_path = os.path.join("include", "IMP",
                                    '' if module == 'kernel' else module, name)
        else:
            out_path = os.path.join("src", module, name)
        files.append((out_path, ip))

    if test:
        files.append(("test/container/test_" + cname + "_restraint.py",
                      path + "/test.py"))
        files.append(("test/container/test_" + cname + "_state.py",
                      path + "/test_state.py"))
    for p in files:
        g = ContainerFileGenerator(p[1])
        g.write(p[0], params)
        all_outputs.append(tools.to_cmake_path(p[0]))
        all_inputs.append(tools.to_cmake_path(p[1]))


def main():
    source = os.path.join(os.path.split(sys.argv[0])[0], "container_templates")
    tools.mkdir(os.path.join("src", "core"), clean=False)
    tools.mkdir(os.path.join("src", "container"), clean=False)
    tools.mkdir(os.path.join("src", "kernel"), clean=False)
    make_one(
        source,
        ("particle", "Particle", "Singleton", "Particle*", "Particle*",
         "Particle*", "Pointer<Particle>",
         "ParticlesTemp", "ParticlesTemp", "Particles",
         "ParticleIndex", "ParticleIndexes", "ParticleIndex"))
    make_one(
        source,
        ("particle_pair", "ParticlePair", "Pair", "ParticlePair",
         "const ParticlePair&", "const ParticlePair", "ParticlePair",
         "ParticlePairsTemp", "ParticlePairsTemp", "ParticlePairs",
         "ParticleIndexPair", "ParticleIndexPairs",
         "const ParticleIndexPair&"))
    make_one(
        source,
        ("particle_triplet", "ParticleTriplet", "Triplet", "ParticleTriplet",
         "const ParticleTriplet&", "const ParticleTriplet", "ParticleTriplet",
         "ParticleTripletsTemp", "ParticleTripletsTemp", "ParticleTriplets",
         "ParticleIndexTriplet", "ParticleIndexTriplets",
         "const ParticleIndexTriplet&"), test=False)
    make_one(
        source,
        ("particle_quad", "ParticleQuad", "Quad", "ParticleQuad",
         "const ParticleQuad&", "const ParticleQuad", "ParticleQuad",
         "ParticleQuadsTemp", "ParticleQuadsTemp", "ParticleQuads",
         "ParticleIndexQuad", "ParticleIndexQuads",
         "const ParticleIndexQuad&"), test=False)
    if True:
        deps = ["${PROJECT_SOURCE_DIR}/tools/build/%s" %
                x[x.find("container_templates"):] for x in all_inputs]
        targets = ["${PROJECT_BINARY_DIR}/%s" % x for x in all_outputs]

        def get_files(module, suffix, prefix, allh):
            ret = []
            for h in allh:
                if (h.endswith(suffix)
                        and os.path.split(os.path.split(h)[0])[1] == module):
                    fname = prefix + os.path.split(h)[1]
                    ret.append(tools.to_cmake_path(fname))
            return ret

        out = """
add_custom_command(OUTPUT %s
  COMMAND "${PYTHON_EXECUTABLE}"
          "${PROJECT_SOURCE_DIR}/tools/build/make_containers.py"
  DEPENDS "${PROJECT_SOURCE_DIR}/tools/build/make_containers.py" %s
  WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
  COMMENT "Making decorator headers")
add_custom_target(IMP-containers ALL DEPENDS %s)
set_property(TARGET "IMP-containers" PROPERTY FOLDER "IMP")

set( IMP_kernel_LIBRARY_EXTRA_DEPENDENCIES ${IMP_kernel_LIBRARY_EXTRA_DEPENDENCIES} IMP-containers)
set( IMP_core_LIBRARY_EXTRA_DEPENDENCIES ${IMP_core_LIBRARY_EXTRA_DEPENDENCIES} IMP-containers)
set( IMP_container_LIBRARY_EXTRA_DEPENDENCIES ${IMP_container_LIBRARY_EXTRA_DEPENDENCIES} IMP-containers)

set( IMP_kernel_PYTHON_EXTRA_DEPENDENCIES ${IMP_kernel_LIBRARY_EXTRA_DEPENDENCIES} IMP-containers)
set( IMP_core_PYTHON_EXTRA_DEPENDENCIES ${IMP_core_LIBRARY_EXTRA_DEPENDENCIES} IMP-containers)
set( IMP_container_PYTHON_EXTRA_DEPENDENCIES ${IMP_container_LIBRARY_EXTRA_DEPENDENCIES} IMP-containers)

set( IMP_kernel_LIBRARY_EXTRA_SOURCES ${IMP_kernel_LIBRARY_EXTRA_SOURCES} %s)
set( IMP_core_LIBRARY_EXTRA_SOURCES ${IMP_core_LIBRARY_EXTRA_SOURCES} %s)
set( IMP_container_LIBRARY_EXTRA_SOURCES ${IMP_container_LIBRARY_EXTRA_SOURCES} %s)

set( IMP_kernel_EXTRA_HEADERS ${IMP_kernel_EXTRA_HEADERS} %s)
set( IMP_core_EXTRA_HEADERS ${IMP_core_EXTRA_HEADERS} %s)
set( IMP_container_EXTRA_HEADERS ${IMP_container_EXTRA_HEADERS} %s)

""" % ("\n   ".join(targets), "\n   ".join(deps),  # noqa: E501
            "\n   ".join(targets),
            "\n   ".join(
                get_files("kernel",
                          ".cpp",
                          "${CMAKE_BINARY_DIR}/src/kernel/",
                          targets)),
            "\n   ".join(
                get_files("core",
                          ".cpp",
                          "${CMAKE_BINARY_DIR}/src/core/",
                          targets)),
            "\n   ".join(
                get_files("container",
                          ".cpp",
                          "${CMAKE_BINARY_DIR}/src/container/",
                          targets)),
            "\n   ".join(get_files("IMP", ".h", "", targets)),
            "\n   ".join(get_files("core", ".h", "", targets)),
            "\n   ".join(get_files("container", ".h", "", targets)))

        g = tools.CMakeFileGenerator()
        g.write(os.path.join(os.path.split(sys.argv[0])[0],
                             "cmake_files", "MakeContainers.cmake"), out)


if __name__ == '__main__':
    main()
