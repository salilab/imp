FIND_PACKAGE(SWIG 1.3.40 REQUIRED)
INCLUDE(${SWIG_USE_FILE})

add_definitions(-DRMF_SWIG_WRAPPER)

if(NOT DEFINED PYTHON_INCLUDE_DIRS)
execute_process(COMMAND python -c "import sys; print(sys.executable)"
                WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
                OUTPUT_VARIABLE python_full_path
                OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND python -c "import sys; print('%d.%d.%d' % sys.version_info[:3])"
                WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
                OUTPUT_VARIABLE python_full_version
                OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE "^([0-9]+\\.[0-9]+).*" "\\1" python_version
       "${python_full_version}")
message(STATUS "Python binary is " ${python_full_path} " (version " ${python_full_version} ")")

FIND_PACKAGE(PythonLibs ${python_full_version} EXACT REQUIRED)
endif()

set(CMAKE_SWIG_OUTDIR "${PROJECT_BINARY_DIR}")
include_directories(BEFORE "${PROJECT_SOURCE_DIR}/swig")
include_directories(${HDF5_INCLUDE_DIRS})

INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_DIRS})

FILE(GLOB rmf_headers "${PROJECT_SOURCE_DIR}/include/RMF/*.h" "${PROJECT_BINARY_DIR}/include/RMF/*.h" "${PROJECT_BINARY_DIR}/include/RMF/decorator/*.h")
FILE(GLOB SWIG_INCLUDES "${PROJECT_SOURCE_DIR}/swig/RMF.*.i")

SET_SOURCE_FILES_PROPERTIES(RMF.i PROPERTIES CPLUSPLUS ON)


SET(SWIG_MODULE_RMF_EXTRA_DEPS ${rmf_headers} ${SWIG_INCLUDES} RMF-decorators
  )

set(CMAKE_SWIG_OUTDIR ${CMAKE_BINARY_DIR}/lib CACHE INTERNAL "" FORCE)
SET(CMAKE_SWIG_FLAGS "-castmode" "-naturalvar" "-Wextra" "-fvirtual"
  "-DPySwigIterator=RMF_PySwigIterator" "-c++"
  "-DSwigPyIterator=RMF_SwigPyIterator"
  "-DRMFEXPORT")

# cmake 3.8 replaced SWIG_ADD_MODULE with SWIG_ADD_LIBRARY
IF (COMMAND SWIG_ADD_LIBRARY)
  SWIG_ADD_LIBRARY(RMF LANGUAGE python TYPE MODULE SOURCES RMF.i)
ELSE()
  SWIG_ADD_MODULE(RMF python RMF.i)
ENDIF()

set_property(TARGET "${SWIG_MODULE_RMF_REAL_NAME}" PROPERTY FOLDER "RMF")
if(APPLE)
  set_target_properties("${SWIG_MODULE_RMF_REAL_NAME}" PROPERTIES LINK_FLAGS "-flat_namespace -undefined suppress")
endif(APPLE)
set_target_properties("${SWIG_MODULE_RMF_REAL_NAME}" PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# Don't link in libpython (that prevents the extension from being used in
# later compatible versions of Python), except on Windows where all DLL
# symbols must be defined
IF(WIN32)
  SWIG_LINK_LIBRARIES(RMF ${PYTHON_LIBRARIES} RMF-lib)
ELSE(WIN32)
  SWIG_LINK_LIBRARIES(RMF RMF-lib)
ENDIF(WIN32)

INSTALL(TARGETS ${SWIG_MODULE_RMF_REAL_NAME} DESTINATION ${CMAKE_INSTALL_PYTHONDIR})
INSTALL(FILES "${CMAKE_BINARY_DIR}/lib/RMF.py" DESTINATION ${CMAKE_INSTALL_PYTHONDIR})


if(RMF_DEPRECATED_BACKENDS)
FILE(GLOB hdf5_headers "${PROJECT_SOURCE_DIR}/include/RMF/HDF5/*.h" "${PROJECT_BINARY_DIR}/include/RMF/HDF5/*.h")


SET_SOURCE_FILES_PROPERTIES(RMF_HDF5.i PROPERTIES CPLUSPLUS ON)
SET(SWIG_MODULE_RMF_HDF5_EXTRA_DEPS ${headers}  ${hdf5_headers} ${SWIG_INCLUDES}
  )

set(CMAKE_SWIG_OUTDIR ${CMAKE_BINARY_DIR}/lib CACHE INTERNAL "" FORCE)
SET(CMAKE_SWIG_FLAGS "-castmode" "-naturalvar" "-Wextra" "-fvirtual"
  "-DPySwigIterator=RMF_HDF5_PySwigIterator" "-c++"
  "-DSwigPyIterator=RMF_HDF5_SwigPyIterator"
  "-DRMFEXPORT")

IF (COMMAND SWIG_ADD_LIBRARY)
  SWIG_ADD_LIBRARY(RMF_HDF5 LANGUAGE python TYPE MODULE SOURCES RMF_HDF5.i)
ELSE()
  SWIG_ADD_MODULE(RMF_HDF5 python RMF_HDF5.i)
ENDIF()

set_property(TARGET "${SWIG_MODULE_RMF_HDF5_REAL_NAME}" PROPERTY FOLDER "RMF")
if(APPLE)
  set_target_properties("${SWIG_MODULE_RMF_HDF5_REAL_NAME}" PROPERTIES LINK_FLAGS "-flat_namespace -undefined suppress")
endif(APPLE)
set_target_properties("${SWIG_MODULE_RMF_HDF5_REAL_NAME}" PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

set(RMF_SWIG _RMF CACHE INTERNAL "" FORCE)


INSTALL(FILES "RMF.i" ${SWIG_INCLUDES} DESTINATION "${CMAKE_INSTALL_SWIGDIR}")


# Don't link in libpython (that prevents the extension from being used in
# later compatible versions of Python), except on Windows where all DLL
# symbols must be defined
IF(WIN32)
  SWIG_LINK_LIBRARIES(RMF_HDF5 ${PYTHON_LIBRARIES} RMF-lib)
ELSE(WIN32)
  SWIG_LINK_LIBRARIES(RMF_HDF5 RMF-lib)
ENDIF(WIN32)
# Apple linkers complain by default if there are undefined symbols
IF(APPLE)
  SET(CMAKE_SHARED_MODULE_CREATE_CXX_FLAGS
    "${CMAKE_SHARED_MODULE_CREATE_CXX_FLAGS} -flat_namespace -undefined suppress")
ENDIF(APPLE)

INSTALL(TARGETS ${SWIG_MODULE_RMF_HDF5_REAL_NAME} DESTINATION ${CMAKE_INSTALL_PYTHONDIR})
INSTALL(FILES "${CMAKE_BINARY_DIR}/lib/RMF_HDF5.py" DESTINATION ${CMAKE_INSTALL_PYTHONDIR})

set(RMF_SWIG _RMF _RMF_HDF5 CACHE INTERNAL "" FORCE)
INSTALL(FILES "RMF_HDF5.i" DESTINATION "${CMAKE_INSTALL_SWIGDIR}")

endif()
