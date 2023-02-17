# Try to find Cereal
#
# The following variables are set:
# cereal_FOUND
# cereal_INCLUDE_DIRS - directories with Cereal headers
# cereal_DEFINITIONS - Cereal compiler flags

find_path(cereal_header_paths_tmp
    NAMES
    cereal.hpp
    PATH_SUFFIXES
    include
    cereal/include
    cereal
	  PATHS
    ${CMAKE_INSTALL_PREFIX}/include
    ${CEREAL_ROOT_DIR}
    ${CEREAL_ROOT_DIR}/include
    ${CEREAL_ROOT_DIR}/cereal/include
    $ENV{CEREAL_ROOT_DIR}
    $ENV{CEREAL_ROOT_DIR}/include
    $ENV{CEREAL_ROOT_DIR}/cereal
    )

get_filename_component(cereal_INCLUDE_DIRS ${cereal_header_paths_tmp} PATH)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(cereal
    REQUIRED_VARS cereal_INCLUDE_DIRS
    )

mark_as_advanced(cereal_FOUND)
