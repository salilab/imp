include_directories(%(includepath)s)
link_directories(%(libpath)s)
add_definitions("-DIMP_EXECUTABLE")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${%(NAME)s_CXX_FLAGS}")

include(Files.cmake)

set(IMP_TEST_ARGUMENTS "--run_quick_test" "--deprecation_exceptions")
set(IMP_LINK_LIBRARIES IMP.%(name)s-lib
    %(modules)s IMP.benchmark-lib
    %(dependencies)s)

imp_add_tests("IMP.%(name)s" ${PROJECT_BINARY_DIR}/benchmark/%(name)s IMP_%(name)s_BENCHMARKS benchmark ${pyfiles} ${cppfiles})
