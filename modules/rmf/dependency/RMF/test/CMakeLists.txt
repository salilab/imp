
file(GLOB python_tests "test/test_*.py")

foreach (test ${python_tests})
   GET_FILENAME_COMPONENT(name ${test} NAME_WE)
  add_test(${name} "python" ${test})
  set_tests_properties(${name}
    PROPERTIES ENVIRONMENT "PYTHONPATH=${PROJECT_BINARY_DIR}")
  #add_dependencies(${name} RMFPython)
endforeach()

file(GLOB cpp_tests "test/test_*.cpp")

foreach (test ${cpp_tests})
   GET_FILENAME_COMPONENT(name ${test} NAME_WE)
   add_executable("${name}" ${test})
   target_link_libraries(${name} RMF)
   set_target_properties(${name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test")
   add_test(${name} "${CMAKE_BINARY_DIR}/test/${name}")
endforeach()
