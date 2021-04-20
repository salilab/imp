include(Files.cmake)
include(${PROJECT_SOURCE_DIR}/cmake_modules/IMPAddTests.cmake)

include_directories(BEFORE ${PROJECT_SOURCE_DIR}/src)
include_directories(BEFORE "${PROJECT_BINARY_DIR}/src")
set(IMP_LINK_LIBRARIES "RMF-lib" CACHE INTERNAL "")
if (DEFINED ENV{PYTHONPATH})
  set(IMP_TESTS_PROPERTIES ENVIRONMENT "PYTHONPATH=${CMAKE_BINARY_DIR}/lib:$ENV{PYTHONPATH}" "PATH=${CMAKE_BINARY_DIR}/bin:$ENV{PATH}")
else()
  set(IMP_TESTS_PROPERTIES ENVIRONMENT "PYTHONPATH=${CMAKE_BINARY_DIR}/lib" "PATH=${CMAKE_BINARY_DIR}/bin:$ENV{PATH}")
endif()
message(set test props ${IMP_TESTS_PROPERTIES})

imp_add_tests("RMF" ${PROJECT_BINARY_DIR}/test RMF_TESTS test ${pyfiles} ${cppfiles})
