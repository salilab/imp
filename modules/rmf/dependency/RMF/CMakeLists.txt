cmake_minimum_required(VERSION 2.8)
project(RMF)

# needs to be in main CMakeLists.txt
enable_testing()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/CMakeModules)

set(RMF_INCLUDE_PATH "" CACHE INTERNAL "" FORCE)

if(POLICY CMP0017)
cmake_policy(SET CMP0017 NEW)
endif(POLICY CMP0017)

include(FindLog4CXX)

if (${Log4CXX_LIBRARY} MATCHES "Log4CXX_LIBRARY-NOTFOUND")
  set(RMF_HAS_LOG4CXX "0" CACHE BOOL "Whether to use Log4CXX")
  message(STATUS "Logging disabled")
  set(Log4CXX_LIBRARY "")
else()
  message(STATUS "Logging enabled")
  set(RMF_HAS_LOG4CXX "1" CACHE BOOL "Whether to use Log4CXX")
  include_directories(${Log4CXX_INCLUDE_DIR})
  link_directories(${Log4CXX_LIBRARY_DIR})
endif()

set(Boost_NO_BOOST_CMAKE ON)
find_package(Boost COMPONENTS system filesystem thread program_options
  REQUIRED)

include(GNUInstallDirs)

include(FindHDF5)
find_package(HDF5 REQUIRED)

include_directories(${HDF5_INCLUDE_DIRS})
set(RMF_INCLUDE_PATH ${RMF_INCLUDE_PATH} ${HDF5_INCLUDE_DIRS}
    CACHE INTERNAL "" FORCE)

include(TestForExtern)

include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR})

set (RMF_VERSION_MAJOR 1)
set (RMF_VERSION_MINOR 0)

set(RMF_HAS_DEBUG_VECTOR 0 CACHE BOOL "Whether to use a bounds checked vector")

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/include/RMF/config.h"
  )

include_directories("${PROJECT_BINARY_DIR}/include"
  "${PROJECT_SOURCE_DIR}/include")

include(data/CMakeLists.txt)
include(include/CMakeLists.txt)

include(src/CMakeLists.txt)
add_subdirectory(bin)
# swig support is a little broken

if (DEFINED CMAKE_INSTALL_PYTHONDIR)
  #message(STATUS "CMAKE_INSTALL_PYTHONDIR defined as ${CMAKE_INSTALL_PYTHONDIR}")
else()
  set(CMAKE_INSTALL_PYTHONDIR "${CMAKE_INSTALL_LIBDIR}/python/site_packages")
endif()

if (DEFINED CMAKE_INSTALL_SWIGDIR)
  #message(STATUS "CMAKE_INSTALL_SWIGDIR defined as ${CMAKE_INSTALL_SWIGDIR}")
else()
  set(CMAKE_INSTALL_SWIGDIR "${CMAKE_INSTALL_PREFIX}/share/RMF/swig")
endif()


add_subdirectory(swig)

include(test/CMakeLists.txt)
include(examples/CMakeLists.txt)
include(benchmark/CMakeLists.txt)
include(doc/CMakeLists.txt)

add_custom_target(RMF_all DEPENDS RMF_benchmarks RMF_bins RMF_wrapper RMF_tests)
add_custom_target(RMF_build DEPENDS RMF_bins RMF_wrapper)
