file(WRITE "${CMAKE_BINARY_DIR}/data/build_info/python-ihm" "ok=True\n")

execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory
                ${CMAKE_BINARY_DIR}/lib/ihm
                RESULT_VARIABLE setup)
if(NOT ${setup} EQUAL 0)
  message(FATAL_ERROR "Failed making ${CMAKE_BINARY_DIR}/lib/ihm directory")
endif()

FILE(GLOB ihmpys
     "${CMAKE_SOURCE_DIR}/modules/core/dependency/python-ihm/ihm/*.py")
if (WIN32)
  foreach(ihmpy ${ihmpys})
    get_filename_component(ihmpyname ${ihmpy} NAME)
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy
                    ${ihmpy} ${CMAKE_BINARY_DIR}/lib/ihm/${ihmpyname}
                    RESULT_VARIABLE setup)
    if(NOT ${setup} EQUAL 0)
      message(FATAL_ERROR "Failed copying ${ihmpy}")
    endif()
  endforeach()
else()
  foreach(ihmpy ${ihmpys})
    get_filename_component(ihmpyname ${ihmpy} NAME)
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
                    ${ihmpy} ${CMAKE_BINARY_DIR}/lib/ihm/${ihmpyname}
                    RESULT_VARIABLE setup)
    if(NOT ${setup} EQUAL 0)
      message(FATAL_ERROR "Failed symlinking ${ihmpy}")
    endif()
  endforeach()
endif()

# Install Python modules
install_deref(${CMAKE_BINARY_DIR}/lib/ihm * ${CMAKE_INSTALL_PYTHONDIR}/ihm)
