if(APPLE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
# to support profiling on mac os
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework CoreFoundation")
list(REMOVE_DUPLICATES CMAKE_CXX_FLAGS)
list(REMOVE_DUPLICATES CMAKE_EXE_LINKER_FLAGS)
endif(APPLE)


if (APPLE)
  execute_process(COMMAND uname -v OUTPUT_VARIABLE DARWIN_VERSION)
  string(REGEX MATCH "[0-9]+" DARWIN_VERSION ${DARWIN_VERSION})
endif()

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" AND "${CMAKE_BUILD_TYPE}" MATCHES "Debug")
  if (NOT APPLE OR DARWIN_VERSION GREATER 12)
    add_definitions("-sanitize=undefined,undefined-trap")
  else()
    message(STATUS "Old clang")
  endif()
endif()