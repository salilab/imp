# Are we running cmake from this directory (out of tree build) ?
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  cmake_minimum_required(VERSION 2.8.3)

  if(POLICY CMP0058)
    cmake_policy(SET CMP0058 NEW)
  endif(POLICY CMP0058)

  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/tools)

  set(SWIG_EXECUTABLE swig CACHE STRING "Swig program")

  find_package(IMP REQUIRED)
  include(${IMP_USE_FILE})

  enable_testing()

  set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

  # Don't set MACOSX_RPATH by default
  set(CMAKE_MACOSX_RPATH 0)

  include_directories(SYSTEM ${IMP_INCLUDE_DIR})
  include_directories("${CMAKE_BINARY_DIR}/include")

  imp_build_module(${CMAKE_SOURCE_DIR})

else()
  include(ModuleBuild.cmake)
endif()
