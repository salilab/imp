
include("Files.cmake")
include(${PROJECT_SOURCE_DIR}/cmake_modules/IMPAddTests.cmake)

include_directories(${PROJECT_SOURCE_DIR}/bin)
set(IMP_LINK_LIBRARIES "RMF-lib" CACHE INTERNAL "")
set(IMP_TESTS_PROPERTIES ENVIRONMENT "PYTHONPATH=${CMAKE_BINARY_DIR}/lib")

imp_add_tests("RMF" ${PROJECT_BINARY_DIR}/examples RMF_TESTS example ${pyfiles} ${cppfiles})
