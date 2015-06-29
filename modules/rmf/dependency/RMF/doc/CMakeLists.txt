# add a target to generate API documentation with Doxygen
include(${PROJECT_SOURCE_DIR}/cmake_modules/FindCurrentDoxygen.cmake)
if(IMP_DOXYGEN_FOUND)
  configure_file(${PROJECT_SOURCE_DIR}/doc/Doxyfile.in ${PROJECT_BINARY_DIR}/doc/Doxyfile @ONLY)
add_custom_target(RMF-doc
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/examples/simple.pdb ${PROJECT_BINARY_DIR}/doc/html/simple.pdb.txt
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/examples/3U7W.pdb ${PROJECT_BINARY_DIR}/doc/html/3U7W.pdb.txt
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/backend/avro/Frame.json ${PROJECT_BINARY_DIR}/doc/html/
        COMMAND ${CMAKE_BINARY_DIR}/bin/rmf_show -v ${PROJECT_SOURCE_DIR}/examples/3U7W.rmf > ${PROJECT_BINARY_DIR}/doc/html/3U7W.txt
        COMMAND ${CMAKE_BINARY_DIR}/bin/rmf_show -v ${PROJECT_SOURCE_DIR}/examples/simple.rmf3 > ${PROJECT_BINARY_DIR}/doc/html/simple.txt
        COMMAND ${CMAKE_BINARY_DIR}/bin/rmf3_dump ${PROJECT_SOURCE_DIR}/examples/3U7W.rmf > ${PROJECT_BINARY_DIR}/doc/html/3U7W.rmf.txt
        COMMAND ${CMAKE_BINARY_DIR}/bin/rmf3_dump ${PROJECT_SOURCE_DIR}/examples/simple.rmf3 > ${PROJECT_BINARY_DIR}/doc/html/simple.rmf3.txt
        COMMAND ${IMP_DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/doc/Doxyfile
        COMMAND cat ${PROJECT_BINARY_DIR}/doc/doc.log
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/doc
        DEPENDS RMF-rmf_show RMF-rmf3_dump
        COMMENT "Generating API documentation with Doxygen" VERBATIM
        )
  set_property(TARGET "RMF-doc" PROPERTY FOLDER "RMF")
else(IMP_DOXYGEN_FOUND)
message(STATUS "RMF doc disabled")
endif(IMP_DOXYGEN_FOUND)
