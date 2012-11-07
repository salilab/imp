
file(GLOB python_tests "examples/*.py")

foreach (test ${python_tests})
   GET_FILENAME_COMPONENT(name ${test} NAME_WE)
  add_test(${name} "python" ${test})
  set_tests_properties(${name}
    PROPERTIES ENVIRONMENT "PYTHONPATH=${PROJECT_BINARY_DIR}")
  #add_dependencies(${name} RMFPython)
endforeach()
