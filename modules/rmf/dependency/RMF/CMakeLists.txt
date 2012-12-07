cmake_minimum_required(VERSION 2.8)
project(RMF)

# needs to be in main CMakeLists.txt
ENABLE_TESTING()

FIND_LIBRARY(avrocpp_LIBRARY NAMES avrocpp)

#message(STATUS ${avrocpp_LIBRARY})

if (${avrocpp_LIBRARY} MATCHES "avrocpp_LIBRARY-NOTFOUND")
message(STATUS "Building avro internally")
ADD_SUBDIRECTORY(AvroCpp)
set(avrocpp_LIBRARY avrocpp)
set(avrocpp_INCLUDE "${CMAKE_SOURCE_DIR}/AvroCpp/")
set(avrogen_PATH "${PROJECT_BINARY_DIR}/AvroCpp/avrogencpp")
set(avrogen_DEPENDENCY "${PROJECT_BINARY_DIR}/AvroCpp/avrogencpp")
else()
message(STATUS "Using external avro")
set(avrogen_PATH "avrogencpp")
endif()


set(Boost_NO_BOOST_CMAKE ON)
find_package(Boost COMPONENTS system filesystem thread program_options
REQUIRED)

include(GNUInstallDirs)

include(FindHDF5)
find_package(HDF5 REQUIRED)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${Boost_INCLUDE_DIR})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR})

set (RMF_VERSION_MAJOR 0)
set (RMF_VERSION_MINOR 9)

set (RMF_USE_DEBUG_VECTOR 0 CACHE BOOL "Whether to use a bounds checked vector")

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/include/RMF/config.h"
  )

include_directories("${PROJECT_BINARY_DIR}/include"
                               "${PROJECT_BINARY_DIR}/src"
                               "${PROJECT_SOURCE_DIR}/src"
                                "${PROJECT_SOURCE_DIR}/include"
                               ${HDF5_INCLUDE_DIRS}
                               ${avrocpp_INCLUDE}
                                ${Boost_INCLUDE_DIR})



include(data/CMakeLists.txt)
include(include/CMakeLists.txt)

include(src/CMakeLists.txt)
ADD_SUBDIRECTORY(bin)
# swig support is a little broken

if (DEFINED CMAKE_INSTALL_PYTHONDIR)
  message(STATUS "CMAKE_INSTALL_PYTHONDIR defined as ${CMAKE_INSTALL_PYTHONDIR}")
else()
  set(CMAKE_INSTALL_PYTHONDIR "${CMAKE_INSTALL_LIBDIR}/python/site_packages")
endif()

if (DEFINED CMAKE_INSTALL_SWIGDIR)
  message(STATUS "CMAKE_INSTALL_SWIGDIR defined as ${CMAKE_INSTALL_SWIGDIR}")
else()
  set(CMAKE_INSTALL_SWIGDIR "${CMAKE_INSTALL_PREFIX}/share/RMF/swig")
endif()


ADD_SUBDIRECTORY(pyext)

include(test/CMakeLists.txt)
include(examples/CMakeLists.txt)
include(benchmark/CMakeLists.txt)
include(doc/CMakeLists.txt)
