cmake_minimum_required(VERSION 2.8)

project(IMP)

include(${PROJECT_SOURCE_DIR}/CMakeModules/ExecuteProcess.cmake)
include(${PROJECT_SOURCE_DIR}/CMakeModules/CheckCompiles.cmake)

if(${PROJECT_SOURCE_DIR} MATCHES ${PROJECT_BINARY_DIR})
   message(ERROR "In-source building not supported: " ${PROJECT_SOURCE_DIR} " == "${PROJECT_BINARY_DIR} )
endif()

imp_execute_process("setup_cmake" ${PROJECT_SOURCE_DIR} COMMAND ${PROJECT_SOURCE_DIR}/tools/build/setup_cmake.py)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/CMakeModules)

enable_testing()

if(POLICY CMP0017)
cmake_policy(SET CMP0017 NEW)
endif(POLICY CMP0017)


set(IMP_SWIG_PATH CACHE STRING "Colon separated list of places for swig to search")
set(SWIG_EXECUTABLE swig CACHE STRING "Swig program")
set(IMP_PYTHON python CACHE STRING "The Python executable that IMP itself will use at runtime (the build system scripts will always use 'python')")
set(IMP_PYTHON_INCLUDE_PATH CACHE STRING "Where to look for python headers")
set(IMP_DISABLED_MODULES CACHE STRING "A colon-separated list of disabled modules")
if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
set(IMP_MAX_CHECKS INTERNAL CACHE STRING "One of NONE, USAGE, INTERNAL")
set(IMP_MAX_LOG VERBOSE CACHE STRING "One of SILENT, PROGRESS, TERSE, VERBOSE")
else()
set(IMP_MAX_CHECKS USAGE CACHE STRING "One of NONE, USAGE, INTERNAL")
set(IMP_MAX_LOG VERBOSE CACHE STRING "One of SILENT, PROGRESS, TERSE, VERBOSE")
endif()
set(IMP_PER_CPP_COMPILATION "" CACHE STRING "A colon-separated list of modules to build one .cpp at a time.")
set(IMP_SPLIT_PYTHON_TESTS FALSE CACHE BOOL "If TRUE, run each Python unittest method as a separate ctest test. (The default is to run the entire Python file as one test.) This gives better reporting of test results, and better isolation of crashing or long running tests, but requires cmake to be rerun every time a test method is added, removed or modified, rather than just whenever files are added or removed.")

string(REGEX MATCHALL "[a-zA-Z]+" percpplist "${IMP_PER_CPP_COMPILATION}")
foreach(m ${percpplist})
message(STATUS ${m} " is percpp")
set(IMP_${m}_IS_PER_CPP 1)
endforeach(m)

if ("${IMP_PYTHON_INCLUDE_PATH}" STREQUAL "")
  message(STATUS "Finding python include path")
  execute_process(COMMAND ${IMP_PYTHON} -c "import distutils.sysconfig; print distutils.sysconfig.get_python_inc();"
                             WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
                             OUTPUT_VARIABLE IMP_PYTHON_INCLUDE_PATH
                             OUTPUT_STRIP_TRAILING_WHITESPACE)
  message(STATUS "Got " ${IMP_PYTHON_INCLUDE_PATH})
endif()

message(STATUS "Determining python version")
execute_process(COMMAND ${IMP_PYTHON} -c "import sys; print '%d.%d' % sys.version_info[:2]"
                WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
                OUTPUT_VARIABLE IMP_PYTHON_VERSION
                OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "Got " ${IMP_PYTHON_VERSION})

include(GNUInstallDirs)
# Add extra installation locations for SWIG .i files and Python code
if(NOT DEFINED CMAKE_INSTALL_SWIGDIR)
  set(CMAKE_INSTALL_SWIGDIR "share/IMP/swig" CACHE PATH "SWIG interface files")
endif()
if(NOT DEFINED CMAKE_INSTALL_PYTHONDIR)
  set(CMAKE_INSTALL_PYTHONDIR "${CMAKE_INSTALL_LIBDIR}/python${IMP_PYTHON_VERSION}/site-packages" CACHE PATH "Python modules")
endif()

include(InstallDeref)

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

include_directories("${PROJECT_BINARY_DIR}/include")

imp_execute_process("setup" ${PROJECT_BINARY_DIR} COMMAND ${PROJECT_SOURCE_DIR}/tools/build/setup.py
                --source=${PROJECT_SOURCE_DIR}
                --disabled=${IMP_DISABLED_MODULES}
                --datapath=${IMP_DATAPATH})

imp_execute_process("setup_containers" ${PROJECT_BINARY_DIR}
                    COMMAND ${PROJECT_SOURCE_DIR}/tools/build/setup_containers.py
                    ${PROJECT_SOURCE_DIR})

imp_execute_process("setup_all" ${PROJECT_BINARY_DIR}
                    COMMAND ${PROJECT_SOURCE_DIR}/tools/build/setup_all.py
                    --source=${PROJECT_SOURCE_DIR})

imp_execute_process("setup_module_alias for compatibility" ${PROJECT_BINARY_DIR}
                    COMMAND ${PROJECT_SOURCE_DIR}/tools/build/setup_module_alias.py
                    --source=${PROJECT_SOURCE_DIR} --module=base --alias=compatibility)

imp_execute_process("setup_module_alias for kernel" ${PROJECT_BINARY_DIR}
                    COMMAND ${PROJECT_SOURCE_DIR}/tools/build/setup_module_alias.py
                    --source=${PROJECT_SOURCE_DIR} --module=kernel "--alias=")

# Don't link in libpython (that prevents the extension from being used in
# later compatible versions of Python), except on Windows where all DLL
# symbols must be defined
IF(WIN32)
  set(SWIG_PYTHON_LIBRARIES ${PYTHON_LIBRARIES} )
ELSE(WIN32)
  set(SWIG_PYTHON_LIBRARIES )
ENDIF(WIN32)
# Apple linkers complain by default if there are undefined symbols
IF(APPLE)
  SET(CMAKE_SHARED_MODULE_CREATE_CXX_FLAGS
    "${CMAKE_SHARED_MODULE_CREATE_CXX_FLAGS} -flat_namespace -undefined suppress")
ENDIF(APPLE)

set(Boost_NO_BOOST_CMAKE ON)
find_package(Boost 1.38.0 COMPONENTS system filesystem thread program_options regex graph REQUIRED)
find_package(Boost 1.38.0 QUIET OPTIONAL_COMPONENTS random)

if(${Boost_RANDOM_LIBRARY} MATCHES ".*NOTFOUND.*")
set(Boost_RANDOM_LIBRARY "" CACHE INTERNAL "" FORCE)
file(WRITE "${PROJECT_BINARY_DIR}/data/build_info/Boost.Random" "ok=False")
else()
file(WRITE "${PROJECT_BINARY_DIR}/data/build_info/Boost.Random" "ok=True")
endif()

include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

set(BOOST.FILESYSTEM_LIBRARIES ${Boost_FILESYSTEM_LIBRARY})
set(BOOST.SYSTEM_LIBRARIES ${Boost_SYSTEM_LIBRARY})
set(BOOST.THREAD_LIBRARIES ${Boost_THREAD_LIBRARY})
set(BOOST.PROGRAMOPTIONS_LIBRARIES ${Boost_PROGRAM_OPTIONS_LIBRARY})
set(BOOST.REGEX_LIBRARIES ${Boost_REGEX_LIBRARY})
set(BOOST.GRAPH_LIBRARIES ${Boost_GRAPH_LIBRARY})
set(BOOST.RANDOM_LIBRARIES ${Boost_RANDOM_LIBRARY})

file(WRITE "${PROJECT_BINARY_DIR}/data/build_info/Boost.FileSystem" "ok=True")
file(WRITE "${PROJECT_BINARY_DIR}/data/build_info/Boost.ProgramOptions" "ok=True")
file(WRITE "${PROJECT_BINARY_DIR}/data/build_info/Boost.Regex" "ok=True")
file(WRITE "${PROJECT_BINARY_DIR}/data/build_info/Boost.Graph" "ok=True")
file(WRITE "${PROJECT_BINARY_DIR}/data/build_info/Boost.System" "ok=True")

# configure modules
file(STRINGS "${PROJECT_BINARY_DIR}/data/build_info/sorted_modules" modules)

foreach(mod ${modules})
  add_subdirectory("${PROJECT_SOURCE_DIR}/modules/${mod}/")
endforeach(mod)

# build modules
file(STRINGS "${PROJECT_BINARY_DIR}/data/build_info/sorted_modules" enabled_modules)

imp_execute_process("setup_swig_dependencies" ${PROJECT_BINARY_DIR}
                    COMMAND ${PROJECT_SOURCE_DIR}/tools/build/setup_swig_deps.py
                    "--swig=${SWIG_EXECUTABLE}")

# Can't set up pyext until we have SWIG dependencies
foreach(mod ${enabled_modules})
  add_subdirectory("${PROJECT_SOURCE_DIR}/modules/${mod}/pyext")
endforeach(mod)

foreach(mod ${enabled_modules})
  add_custom_target(IMP.${mod} DEPENDS
  imp_${mod}
  imp_${mod}_python
  imp_${mod}_bins
  imp_${mod}_tests
  imp_${mod}_benchmarks
  imp_${mod}_examples)
endforeach(mod)


file(STRINGS "${PROJECT_BINARY_DIR}/data/build_info/applications" applications)
foreach(app ${applications})
  add_subdirectory("${PROJECT_SOURCE_DIR}/applications/${app}")
endforeach(app)

imp_execute_process("setup_imppy" ${PROJECT_BINARY_DIR}
                    COMMAND ${PROJECT_SOURCE_DIR}/tools/build/setup_imppy.py
                    "--python_path=${PROJECT_BINARY_DIR}/lib:${IMP_PYTHONPATH}"
                    "--ld_path=${PROJECT_BINARY_DIR}/lib:${IMP_LDPATH}"
                    "--python=${IMP_PYTHON}"
                    "--precommand="
                    "--propagate=yes"
                    "--output=setup_environment.sh"
                    "--path=${PROJECT_BINARY_DIR}/bin:${IMP_PATH}")

find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
add_custom_command(OUTPUT "${PROJECT_BINARY_DIR}/doc/htmp/index.html"
                   COMMAND ${DOXYGEN_EXECUTABLE} doxygen/Doxyfile.xml
                   COMMAND ${PROJECT_SOURCE_DIR}/tools/build/make_cross_refs.py
                   COMMAND ${DOXYGEN_EXECUTABLE} doxygen/Doxyfile.html
   WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
   COMMENT "Building documentation")

add_custom_target("imp-doc" DEPENDS "${PROJECT_BINARY_DIR}/doc/htmp/index.html")
endif()

# Install headers
INSTALL(FILES ${CMAKE_BINARY_DIR}/include/IMP.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install_deref(${CMAKE_BINARY_DIR}/include/IMP *.h ${CMAKE_INSTALL_INCLUDEDIR}/IMP)

# Install Python modules
install_deref(${CMAKE_BINARY_DIR}/lib/IMP * ${CMAKE_INSTALL_PYTHONDIR}/IMP)

# Install data
install_deref(${CMAKE_BINARY_DIR}/data * ${CMAKE_INSTALL_DATADIR}/IMP)

# Install SWIG .i files
install_deref(${CMAKE_BINARY_DIR}/swig *.i ${CMAKE_INSTALL_SWIGDIR})

# Install examples
install_deref(${CMAKE_BINARY_DIR}/doc/examples *
              ${CMAKE_INSTALL_DOCDIR}/examples)
