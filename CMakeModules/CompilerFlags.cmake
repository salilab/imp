if(${IMP_BUILD} MATCHES "DEBUG")
set(CMAKE_BUILD_TYPE Debug)
set(base_flags ${CMAKE_CXX_FLAGS_DEBUG})
else()
set(CMAKE_BUILD_TYPE Release)
set(base_flags ${CMAKE_CXX_FLAGS_RELEASE})
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(IMP_USE_CUSTOM_CXX_FLAGS 1 CACHE INTERNAL "" FORCE)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(IMP_USE_CUSTOM_CXX_FLAGS 1 CACHE INTERNAL "" FORCE)
  set(version ${CMAKE_CXX_COMPILER_VERSION})
  if("${version}" STREQUAL "")
    execute_process (COMMAND ${CMAKE_CXX_COMPILER} --version
    OUTPUT_VARIABLE rawversion
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_STRIP_TRAILING_WHITESPACE)
    message(STATUS "raw version " ${CMAKE_CXX_COMPILER} "${rawversion}")
    string(REGEX MATCH "4\\.[0-9]" version ${rawversion})
  endif()
else()
set(IMP_USE_CUSTOM_CXX_FLAGS 0 CACHE INTERNAL "" FORCE)
endif()

if ("${IMP_USE_CUSTOM_CXX_FLAGS}")
  message(STATUS "Compiler " ${version} " of " "${CMAKE_CXX_COMPILER_ID}")
  message(STATUS "Base flags are ${base_flags} for ${IMP_BUILD}")
  execute_process(COMMAND ${PROJECT_SOURCE_DIR}/tools/build/setup_compiler.py
    --compiler=${CMAKE_CXX_COMPILER_ID}
    --flags=${base_flags}
    --output=lib
    --build=${CMAKE_BUILD_TYPE}
    --version=${version}
    RESULT_VARIABLE setup
    OUTPUT_VARIABLE IMP_LIB_CXX_FLAGS
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  if( ${setup})
    message(FATAL_ERROR "Failed to run compiler setup")
  endif()
  execute_process (COMMAND ${PROJECT_SOURCE_DIR}/tools/build/setup_compiler.py
    --compiler=${CMAKE_CXX_COMPILER_ID}
    --flags=${base_flags}
    --output=bin
    --build=${CMAKE_BUILD_TYPE}
    --version=${version}
    RESULT_VARIABLE setup
    OUTPUT_VARIABLE IMP_BIN_CXX_FLAGS
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  if( ${setup})
    message(FATAL_ERROR "Failed to run compiler setup")
  endif()
  execute_process(COMMAND ${PROJECT_SOURCE_DIR}/tools/build/setup_compiler.py
    --compiler=${CMAKE_CXX_COMPILER_ID}
    --flags=${base_flags}
    --output=python
    --build=${CMAKE_BUILD_TYPE}
    --version=${version}
    RESULT_VARIABLE setup
    OUTPUT_VARIABLE IMP_PYTHON_CXX_FLAGS
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  if( ${setup})
    message(FATAL_ERROR "Failed to run compiler setup")
  endif()
  message(STATUS "Lib cxx flags are ${IMP_LIB_CXX_FLAGS}")
  message(STATUS "Bin cxx flags are ${IMP_BIN_CXX_FLAGS}")
  message(STATUS "Python cxx flags are ${IMP_PYTHON_CXX_FLAGS}")
else()

endif()