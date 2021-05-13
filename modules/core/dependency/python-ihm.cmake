set(IMP_USE_SYSTEM_IHM off CACHE BOOL "Use an external (system) copy of python-ihm, rather than that bundled with IMP.")

file(WRITE "${CMAKE_BINARY_DIR}/build_info/python-ihm" "ok=True\n")

if(EXISTS ${CMAKE_BINARY_DIR}/lib/ihm
   AND IS_SYMLINK ${CMAKE_BINARY_DIR}/lib/ihm)
  file(REMOVE ${CMAKE_BINARY_DIR}/lib/ihm)
endif()

# We need ihm_format.h to build IMP's mmCIF reader, even if we're using
# external python-ihm
set(PYTHON-IHM_INCLUDE_PATH ${CMAKE_SOURCE_DIR}/modules/core/dependency/python-ihm/src/ CACHE INTERNAL "" FORCE)

if(IMP_USE_SYSTEM_IHM)
# Clean up after a non-system IHM build
execute_process(COMMAND ${CMAKE_COMMAND} -E rm -rf
                ${CMAKE_BINARY_DIR}/lib/ihm)

else(IMP_USE_SYSTEM_IHM)

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

# Build C extension

# We used add_definitions to add C++11 compiler flags, but that will be applied
# to C code too, and some compilers treat that as an error, so remove
if(IMP_CXX11_FLAGS)
  remove_definitions(${IMP_CXX11_FLAGS})
endif()

# Our use of strdup, strerror should be safe - no need for the Windows
# compiler to warn about it; we want to use the POSIX name for strdup too
if(WIN32)
  add_definitions("-D_CRT_SECURE_NO_WARNINGS")
  add_definitions("-D_CRT_NONSTDC_NO_WARNINGS")
endif()

set(swig_i
    "${CMAKE_SOURCE_DIR}/modules/core/dependency/python-ihm/src/ihm_format.i")
set(ext_c
    "${CMAKE_SOURCE_DIR}/modules/core/dependency/python-ihm/src/ihm_format.c")
set(wrap_c "${CMAKE_BINARY_DIR}/src/core_swig/ihm_format_wrap.c")
add_custom_command(OUTPUT ${wrap_c}
	           COMMAND ${SWIG_EXECUTABLE} "-python" "-noproxy"
		           "-keyword" "-nodefaultctor" "-nodefaultdtor"
                           "-o" ${wrap_c} ${swig_i}
		   DEPENDS ${swig_i} ${ext_c}
		   WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
		   COMMENT "Running SWIG on ihm.format")
include_directories(${PYTHON_INCLUDE_DIRS}
                    ${CMAKE_SOURCE_DIR}/modules/core/dependency/python-ihm/src/)
add_library(ihm-python MODULE ${wrap_c} ${ext_c})
# Apple linkers complain by default if there are undefined symbols
if(APPLE)
  set_target_properties(ihm-python
                 PROPERTIES LINK_FLAGS "-flat_namespace -undefined suppress")
endif(APPLE)

target_link_libraries(ihm-python ${IMP_SWIG_LIBRARIES})

set_target_properties(ihm-python PROPERTIES PREFIX ""
                      OUTPUT_NAME _format
                      LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/ihm)
set_property(TARGET "ihm-python" PROPERTY FOLDER "ihm")

if(WIN32 AND NOT CYGWIN)
  set_target_properties(ihm-python PROPERTIES SUFFIX ".pyd")
endif()

# Install C extension
install(TARGETS ihm-python DESTINATION ${CMAKE_INSTALL_PYTHONDIR}/ihm)
endif(IMP_USE_SYSTEM_IHM)
