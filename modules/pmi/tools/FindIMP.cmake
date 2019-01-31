#
# The following module is based on FindVTK.cmake
#

# - Find an IMP installation or binary tree.
# The following variables are set if IMP is found.  If IMP is not
# found, IMP_FOUND is set to false.
#
#  IMP_FOUND         - Set to true when IMP is found.
#  IMP_USE_FILE      - CMake file to use IMP.
#

# Construct consistent error messages for use below.
set(IMP_DIR_DESCRIPTION "directory containing IMPConfig.cmake. This is either the cmake binary directory where IMP was configured or PREFIX/lib/cmake/IMP if you installed IMP.")
set(IMP_DIR_MESSAGE     "IMP not found.  Set the IMP_DIR cmake variable or environment variable to the ${IMP_DIR_DESCRIPTION}")
 
if ( NOT IMP_DIR )
  
  # Get the system search path as a list.
  if(UNIX)
    string(REGEX MATCHALL "[^:]+" IMP_DIR_SEARCH1 "$ENV{PATH}")
  else()
    string(REGEX REPLACE "\\\\" "/" IMP_DIR_SEARCH1 "$ENV{PATH}")
  endif()
  
  string(REGEX REPLACE "/;" ";" IMP_DIR_SEARCH2 "${IMP_DIR_SEARCH1}")

  # Construct a set of paths relative to the system search path.
  set(IMP_DIR_SEARCH "")
  
  foreach(dir ${IMP_DIR_SEARCH2})
  
    set(IMP_DIR_SEARCH ${IMP_DIR_SEARCH} ${dir}/../lib/cmake/IMP )
      
  endforeach()


  #
  # Look for an installation or build tree.
  #
  find_path(IMP_DIR IMPConfig.cmake

    # Look for an environment variable IMP_DIR.
    $ENV{IMP_DIR}

    # Look in places relative to the system executable search path.
    ${IMP_DIR_SEARCH}

    # Look in standard UNIX install locations.
    /usr/local/lib/IMP
    /usr/local/lib/cmake/IMP
    /usr/lib64/IMP
    /usr/lib64/cmake/IMP
    /usr/lib/IMP
    /usr/lib/cmake/IMP

    # Help the user find it if we cannot.
    DOC "The ${IMP_DIR_DESCRIPTION}"
  )
  
endif()

if ( IMP_DIR )
  
  if ( EXISTS "${IMP_DIR}/IMPConfig.cmake" )
    include( "${IMP_DIR}/IMPConfig.cmake" )
    set( IMP_FOUND TRUE )
  endif()

endif()

if( NOT IMP_FOUND)
  if(IMP_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR ${IMP_DIR_MESSAGE})
  else()
    if(NOT IMP_FIND_QUIETLY)
      MESSAGE(STATUS ${IMP_DIR_MESSAGE})
    endif()
  endif()
endif()
