
file(GLOB python_tests "examples/*.py")

foreach (test ${python_tests})
   GET_FILENAME_COMPONENT(name ${test} NAME_WE)
  add_test("RMF.${name}" "python" ${test})
  set_tests_properties("RMF.${name}"
    PROPERTIES ENVIRONMENT "PYTHONPATH=${PROJECT_BINARY_DIR}")
  set_tests_properties("RMF.${name}" PROPERTIES LABELS "RMF;example")
  #add_dependencies(${name} RMFPython)
endforeach()
