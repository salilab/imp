include_directories(%(includepath)s)
link_directories(%(libpath)s)

include(Files.cmake)

set(IMP_TEST_ARGUMENTS "--run_quick_test" "--deprecation_exceptions")
set(IMP_LINK_LIBRARIES IMP.%(name)s-lib
    %(modules)s
    %(dependencies)s)

imp_add_tests("IMP.%(name)s" ${PROJECT_BINARY_DIR}/doc/examples/%(name)s IMP_%(name)s_EXAMPLES example ${pyfiles} ${cppfiles})
