file(WRITE "${CMAKE_BINARY_DIR}/data/build_info/python-ihm" "ok=True\n")

if (WIN32)
  execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory
                  ${CMAKE_SOURCE_DIR}/modules/core/dependency/python-ihm/ihm
                  ${CMAKE_BINARY_DIR}/lib/ihm
                  RESULT_VARIABLE setup)
else()
  execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
                  ${CMAKE_SOURCE_DIR}/modules/core/dependency/python-ihm/ihm
                  ${CMAKE_BINARY_DIR}/lib/ihm
                  RESULT_VARIABLE setup)
endif()
if(NOT ${setup} EQUAL 0)
   message(FATAL_ERROR "Failed making ${CMAKE_BINARY_DIR}/lib/ihm directory")
endif()

# Install Python modules
install_deref(${CMAKE_BINARY_DIR}/lib/ihm * ${CMAKE_INSTALL_PYTHONDIR}/ihm)
