#[=======================================================================[.rst:
FindRMF
-------

Try to find RMF

Result Variables
^^^^^^^^^^^^^^^^

This module defines the following variables:

``RMF_FOUND``
  system has RMF
``RMF_INCLUDE_PATH``
  the RMF include directory
``RMF_SWIG_PATH``
  the directory containing SWIG (.i) files for RMF
``RMF_LIBRARY``
  Link this to use RMF
``RMF_VERSION_STRING``
  the version of RMF found


#]=======================================================================]


find_path(RMF_INCLUDE_PATH RMF.h PATH_SUFFIXES include)
find_path(RMF_SWIG_PATH RMF.i PATH_SUFFIXES share/RMF/swig)
if (NOT RMF_LIBRARY)
  find_library(RMF_LIBRARY NAMES RMF PATH_SUFFIXES lib)
endif()

if (RMF_INCLUDE_PATH AND EXISTS "${RMF_INCLUDE_PATH}/RMF/config.h")
  file(STRINGS "${RMF_INCLUDE_PATH}/RMF/config.h" RMF_MAJOR_H REGEX "#define RMF_VERSION_MAJOR +([0-9]+)")
  file(STRINGS "${RMF_INCLUDE_PATH}/RMF/config.h" RMF_MINOR_H REGEX "#define RMF_VERSION_MINOR +([0-9]+)")
  string(REGEX REPLACE " *#define RMF_VERSION_MAJOR +([0-9]+) *" "\\1" RMF_VERSION_MAJOR "${RMF_MAJOR_H}")
  string(REGEX REPLACE " *#define RMF_VERSION_MINOR +([0-9]+) *" "\\1" RMF_VERSION_MINOR "${RMF_MINOR_H}")
  set(RMF_VERSION_STRING "${RMF_VERSION_MAJOR}.${RMF_VERSION_MINOR}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(RMF
        REQUIRED_VARS RMF_LIBRARY RMF_INCLUDE_PATH RMF_SWIG_PATH
        VERSION_VAR RMF_VERSION_STRING)
