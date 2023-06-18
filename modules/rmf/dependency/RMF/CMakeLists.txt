cmake_minimum_required(VERSION 2.8.12)
project(RMF)

# needs to be in main CMakeLists.txt
enable_testing()

# Default Python3_FIND_STRATEGY to LOCATION
if(POLICY CMP0094)
cmake_policy(SET CMP0094 NEW)
endif(POLICY CMP0094)

# Use new-style variables/escapes
if(POLICY CMP0053)
cmake_policy(SET CMP0053 NEW)
endif(POLICY CMP0053)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake_modules)
include(cmake_modules/IMPFindPython.cmake)
imp_find_python()

execute_process(COMMAND ${PYTHON_EXECUTABLE} setup_git.py
                  RESULT_VARIABLE setup
                  OUTPUT_VARIABLE toutput
                  ERROR_VARIABLE error
                  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT ${setup} EQUAL 0)
   message(FATAL_ERROR " Failed to run setup_git.py: ${setup}; ${error}")
endif()

# Don't deref quoted variables in if()
if(POLICY CMP0054)
cmake_policy(SET CMP0054 NEW)
endif(POLICY CMP0054)

# Use new method of handling Ninja custom command byproducts
if(POLICY CMP0058)
  cmake_policy(SET CMP0058 NEW)
endif(POLICY CMP0058)

# Use consistent SWIG target names across all supported cmake versions
if(POLICY CMP0078)
  cmake_policy(SET CMP0078 OLD)
endif(POLICY CMP0078)

# Quiet warning about UseSWIG honoring SWIG_MODULE_NAME (which we don't use)
if(POLICY CMP0086)
  cmake_policy(SET CMP0086 NEW)
endif(POLICY CMP0086)

# C++11 can use the boost version
set(Boost_NO_BOOST_CMAKE ON)
find_package(Boost COMPONENTS system filesystem thread program_options iostreams
  REQUIRED)

include(FindHDF5)
find_package(HDF5 1.8)

if("${HDF5_INCLUDE_DIRS}" MATCHES .*NOTFOUND)
set(RMF_DEPRECATED_BACKENDS 0 CACHE BOOL "Whether to build deprecated backends.")
set(HDF5_LIBRARIES "" CACHE INTERNAL "" FORCE)
set(HDF5_INCLUDE_DIRS "" CACHE INTERNAL "" FORCE)
else()
set(RMF_DEPRECATED_BACKENDS 1 CACHE BOOL "Whether to build deprecated backends.")
endif()

if(RMF_DEPRECATED_BACKENDS)
message(STATUS "Deprecated backends (and hdf5) are enabled")
else()
message(STATUS "Deprecated backends (and hdf5) are disabled")
endif()

include(cmake_modules/IMPExecuteProcess.cmake)
include(cmake_modules/FindLog4CXX.cmake)
include(cmake_modules/IMPFindC++11.cmake)
include(cmake_modules/IMPFindCompilerFlags.cmake)

if(RMF_DEPRECATED_BACKENDS)
imp_execute_process("Making RMF/HDF5.h" ${PROJECT_SOURCE_DIR}
  COMMAND ${PYTHON_EXECUTABLE}
  "${PROJECT_SOURCE_DIR}/tools/dev_tools/make_all_header.py"
  ${PROJECT_BINARY_DIR}/include/RMF/HDF5.h RMF/HDF5
  ${PROJECT_SOURCE_DIR}/include/RMF/HDF5)
endif()

imp_execute_process("Making RMF.h" ${PROJECT_SOURCE_DIR}
  COMMAND ${PYTHON_EXECUTABLE}
  "${PROJECT_SOURCE_DIR}/tools/dev_tools/make_all_header.py"
  ${PROJECT_BINARY_DIR}/include/RMF.h RMF ${PROJECT_SOURCE_DIR}/include/RMF)

message(STATUS "Running setup_cmake.py")
imp_execute_process("Running setup_cmake.py" ${PROJECT_SOURCE_DIR}
  COMMAND ${PYTHON_EXECUTABLE}
  ${PROJECT_SOURCE_DIR}/tools/dev_tools/setup_cmake.py)

include_directories(BEFORE "${PROJECT_BINARY_DIR}/include"
  "${PROJECT_SOURCE_DIR}/include")

set(RMF_INCLUDE_PATH "" CACHE INTERNAL "" FORCE)

# make sure it initialized
set(RMF_TESTS "" CACHE INTERNAL "" FORCE)

if(POLICY CMP0017)
cmake_policy(SET CMP0017 NEW)
endif(POLICY CMP0017)

# Don't set MACOSX_RPATH by default
set(CMAKE_MACOSX_RPATH 0)

if (${Log4CXX_LIBRARY} MATCHES "Log4CXX_LIBRARY-NOTFOUND")
  set(RMF_HAS_LOG4CXX "0" CACHE BOOL "Whether to use Log4CXX" FORCE)
  message(STATUS "Logging disabled")
  set(Log4CXX_LIBRARY "")
else()
  message(STATUS "Logging enabled")
  set(RMF_HAS_LOG4CXX "1" CACHE BOOL "Whether to use Log4CXX" FORCE)
  include_directories(${Log4CXX_INCLUDE_DIR})
  link_directories(${Log4CXX_LIBRARY_DIR})
endif()

if(${PYTHON_NUMPY_FOUND})
  set(RMF_HAS_NUMPY "1" CACHE BOOL "Whether to include numpy support" FORCE)
else()
  set(RMF_HAS_NUMPY "0" CACHE BOOL "Whether to include numpy support" FORCE)
endif()

include(GNUInstallDirs)


include_directories(${HDF5_INCLUDE_DIRS})
set(RMF_INCLUDE_PATH ${RMF_INCLUDE_PATH} ${HDF5_INCLUDE_DIRS}
    CACHE INTERNAL "" FORCE)

include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

# Version information
set (RMF_VERSION_MAJOR 1)
set (RMF_VERSION_MINOR 5)
set (RMF_VERSION_MICRO 1)

set(RMF_SOVERSION "${RMF_VERSION_MAJOR}.${RMF_VERSION_MINOR}" CACHE INTERNAL "" FORCE)
set(RMF_HAS_DEBUG_VECTOR 0 CACHE BOOL "Whether to use a bounds checked vector")

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/include/RMF/config.h"
  )

add_subdirectory(data)
add_subdirectory(include)

add_subdirectory(src)
add_subdirectory(bin)

add_subdirectory(plugins/vmd/)
add_subdirectory(plugins/pymol/)

# swig support is a little broken

if (DEFINED CMAKE_INSTALL_PYTHONDIR)
  #message(STATUS "CMAKE_INSTALL_PYTHONDIR defined as ${CMAKE_INSTALL_PYTHONDIR}")
else()
  set(CMAKE_INSTALL_PYTHONDIR "${CMAKE_INSTALL_LIBDIR}/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages")
endif()

if (DEFINED CMAKE_INSTALL_SWIGDIR)
  #message(STATUS "CMAKE_INSTALL_SWIGDIR defined as ${CMAKE_INSTALL_SWIGDIR}")
else()
  set(CMAKE_INSTALL_SWIGDIR "${CMAKE_INSTALL_PREFIX}/share/RMF/swig")
endif()


add_subdirectory(swig)

add_subdirectory(test)
add_subdirectory(examples)
add_subdirectory(benchmark)
add_subdirectory(doc)

add_custom_target(RMF DEPENDS ${RMF_BINS} ${RMF_SWIG} ${RMF_TESTS} ${RMF_VMD} RMF-decorators)
set_property(TARGET "RMF" PROPERTY FOLDER "RMF")
