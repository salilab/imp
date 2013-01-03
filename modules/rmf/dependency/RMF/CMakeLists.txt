cmake_minimum_required(VERSION 2.8)
project(RMF)

# needs to be in main CMakeLists.txt
enable_testing()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules)

include(FindAvro)

if ((${avrocpp_LIBRARY} MATCHES "avrocpp_LIBRARY-NOTFOUND")
    OR (${avrogencpp_PROGRAM} MATCHES "avrogencpp_PROGRAM-NOTFOUND"))
  if ((NOT ${avrocpp_LIBRARY} MATCHES "avrocpp_LIBRARY-NOTFOUND")
    OR (NOT ${avrogencpp_PROGRAM} MATCHES "avrogencpp_PROGRAM-NOTFOUND"))
    message(SEND_ERROR "A partial avro install exists on the system, this is a bad thing: "
     ${avrocpp_LIBRARY} " " ${avrogencpp_PROGRAM} " " ${avrocpp_HEADER})
  endif()
  message(STATUS "Using internal Avro")
  add_subdirectory(AvroCpp)
  set(avrogencpp_PROGRAM "${PROJECT_BINARY_DIR}/AvroCpp/avrogencpp")
  set(avrogen_DEPENDENCY "${PROJECT_BINARY_DIR}/AvroCpp/avrogencpp")
  include_directories("${PROJECT_SOURCE_DIR}/AvroCpp")
  set(avrocpp_LIBRARY avrocpp)
else()
  message(STATUS "Using external Avro at " ${avrocpp_HEADER})
endif()

include(FindLog4CXX)

if (${Log4CXX_LIBRARY} MATCHES "Log4CXX_LIBRARY-NOTFOUND")
  set(RMF_USE_LOG4CXX, "0")
  message(STATUS "Logging disabled")
else()
  message(STATUS "Logging enabled")
  set(RMF_USE_LOG4CXX, "1")
endif()

set(Boost_NO_BOOST_CMAKE ON)
find_package(Boost COMPONENTS system filesystem thread program_options
  REQUIRED)

include(GNUInstallDirs)

include(FindHDF5)
find_package(HDF5 REQUIRED)

include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

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
  "${PROJECT_SOURCE_DIR}/include"
  ${HDF5_INCLUDE_DIRS})

include(data/CMakeLists.txt)
include(include/CMakeLists.txt)

include(src/CMakeLists.txt)
add_subdirectory(bin)
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


add_subdirectory(swig)

include(test/CMakeLists.txt)
include(examples/CMakeLists.txt)
include(benchmark/CMakeLists.txt)
include(doc/CMakeLists.txt)
