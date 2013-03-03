configure_file (
  "doc/json.dox.in"
  "${PROJECT_BINARY_DIR}/doc/json.dox"
  )


# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
  configure_file(${PROJECT_SOURCE_DIR}/doc/Doxyfile.in ${PROJECT_BINARY_DIR}/doc/Doxyfile @ONLY)
add_custom_target(rmf-doc
${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/doc/Doxyfile
WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/doc
COMMENT "Generating API documentation with Doxygen" VERBATIM
)
endif(DOXYGEN_FOUND)
