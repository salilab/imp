FIND_PACKAGE(SWIG REQUIRED)
INCLUDE(${SWIG_USE_FILE})

if(NOT DEFINED PYTHON_INCLUDE_DIRS)
execute_process(COMMAND python -c "import sys; print '%d.%d.%d' % sys.version_info[:3]"
                WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
                OUTPUT_VARIABLE python_full_version
                OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE "^([0-9]+\\.[0-9]+).*" "\\1" python_version
       "${python_full_version}")
message(STATUS "Python version is " ${python_version})

FIND_PACKAGE(PythonLibs ${python_full_version} EXACT REQUIRED)
endif()

message(STATUS ${PYTHON_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_DIRS})

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/swig)

SET(CMAKE_SWIG_FLAGS "-castmode" "-naturalvar" "-Wextra" "-fvirtual"
  "-DPySwigIterator=RMF_PySwigIterator" "-c++"
  "-DSwigPyIterator=RMF_SwigPyIterator"
  "-DRMFEXPORT")
set(CMAKE_SWIG_OUTDIR "${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_BINARY_DIR}/include"
  "${PROJECT_SOURCE_DIR}/include" "${PROJECT_SOURCE_DIR}/swig" ${HDF5_INCLUDE_DIRS})

FILE(GLOB rmf_headers "${PROJECT_SOURCE_DIR}/include/RMF/*.h" "${PROJECT_BINARY_DIR}/include/RMF/*.h")
FILE(GLOB headers "${PROJECT_SOURCE_DIR}/include/RMF/HDF5/*.h" "${PROJECT_BINARY_DIR}/include/RMF/HDF5/*.h")
FILE(GLOB SWIG_INCLUDES "${PROJECT_SOURCE_DIR}/swig/RMF.*.i")

SET_SOURCE_FILES_PROPERTIES(RMF.i PROPERTIES CPLUSPLUS ON)
SET(SWIG_MODULE_RMF_EXTRA_DEPS
  ${rmf_headers}  ${headers} ${SWIG_INCLUDES}
  )

SWIG_ADD_MODULE(RMF python RMF.i)

# Don't link in libpython (that prevents the extension from being used in
# later compatible versions of Python), except on Windows where all DLL
# symbols must be defined
IF(WIN32)
  SWIG_LINK_LIBRARIES(RMF ${PYTHON_LIBRARIES} RMF)
ELSE(WIN32)
  SWIG_LINK_LIBRARIES(RMF RMF)
ENDIF(WIN32)
# Apple linkers complain by default if there are undefined symbols
IF(APPLE)
  SET(CMAKE_SHARED_MODULE_CREATE_CXX_FLAGS
    "${CMAKE_SHARED_MODULE_CREATE_CXX_FLAGS} -flat_namespace -undefined suppress")
ENDIF(APPLE)

INSTALL(TARGETS ${SWIG_MODULE_RMF_REAL_NAME} DESTINATION ${CMAKE_INSTALL_PYTHONDIR})
INSTALL(FILES "${PROJECT_BINARY_DIR}/RMF.py" DESTINATION ${CMAKE_INSTALL_PYTHONDIR})


SET_SOURCE_FILES_PROPERTIES(RMF_HDF5.i PROPERTIES CPLUSPLUS ON)
SET(SWIG_MODULE_RMF_HDF5_EXTRA_DEPS
  ${headers}  ${hdf5_headers} ${SWIG_INCLUDES}
  )

SWIG_ADD_MODULE(RMF_HDF5 python RMF_HDF5.i)

# Don't link in libpython (that prevents the extension from being used in
# later compatible versions of Python), except on Windows where all DLL
# symbols must be defined
IF(WIN32)
  SWIG_LINK_LIBRARIES(RMF_HDF5 ${PYTHON_LIBRARIES} RMF)
ELSE(WIN32)
  SWIG_LINK_LIBRARIES(RMF_HDF5 RMF)
ENDIF(WIN32)
# Apple linkers complain by default if there are undefined symbols
IF(APPLE)
  SET(CMAKE_SHARED_MODULE_CREATE_CXX_FLAGS
    "${CMAKE_SHARED_MODULE_CREATE_CXX_FLAGS} -flat_namespace -undefined suppress")
ENDIF(APPLE)

INSTALL(TARGETS ${SWIG_MODULE_RMF_HDF5_REAL_NAME} DESTINATION ${CMAKE_INSTALL_PYTHONDIR})
INSTALL(FILES "${PROJECT_BINARY_DIR}/RMF_HDF5.py" DESTINATION ${CMAKE_INSTALL_PYTHONDIR})


FILE(GLOB swigfiles "*.i")

INSTALL(FILES ${swigfiles} DESTINATION "${CMAKE_INSTALL_SWIGDIR}")

add_custom_target("RMF_wrapper" ALL DEPENDS _RMF _RMF_HDF5)
