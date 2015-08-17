set(VMDPLUGIN_INCLUDE_DIR ${VMDPLUGIN_INCLUDE_DIR} CACHE STRING "Path to vmd plugin headers")

find_path(VMDPLUGIN_INCLUDE_PATH
          NAMES molfile_plugin.h
          PATHS ${VMDPLUGIN_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/plugins/vmd/include
          )

if(${VMDPLUGIN_INCLUDE_PATH} MATCHES "VMDPLUGIN_INCLUDE_PATH-NOTFOUND")
message(STATUS "Compilation of VMD plugin disabled. Define VMDPLUGIN_INCLUDE_DIR to point to the include path for the header.")
set(RMF_VMD "" CACHE INTERNAL "" FORCE)
else()
include_directories(${VMDPLUGIN_INCLUDE_PATH})

message(STATUS "Using VMD headers found in ${VMDPLUGIN_INCLUDE_PATH}")

include_directories(BEFORE ".")
add_library(RMF-vmd MODULE "vmdplugin.cpp")
if(WIN32)
  set(DSO_SUFFIX ".dll")
else()
  set(DSO_SUFFIX ".so")
endif()
set_target_properties("RMF-vmd" PROPERTIES OUTPUT_NAME "rmfplugin"
                                PREFIX ""
                                SUFFIX "${DSO_SUFFIX}"
                                COMPILE_DEFINITIONS "VMDPLUGIN_EXPORTS"
                                RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set_property(TARGET "RMF-vmd" PROPERTY FOLDER "RMF")


target_link_libraries(RMF-vmd
                      RMF-lib
                      ${Boost_FILESYSTEM_LIBRARY}
                      ${Boost_SYSTEM_LIBRARY}
                      ${Boost_THREAD_LIBRARY}
                      ${HDF5_LIBRARIES}
                      ${Log4CXX_LIBRARY}
                      )
set(RMF_VMD "RMF-vmd" CACHE INTERNAL "" FORCE)
endif()
