FIND_PACKAGE(SWIG REQUIRED)
INCLUDE(${SWIG_USE_FILE})

# We only work with Python 2 for now (not Python 3)
SET(Python_ADDITIONAL_VERSIONS 2.7 2.6 2.5 2.4)
FIND_PACKAGE(PythonLibs)
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/swig)

SET(CMAKE_SWIG_FLAGS "-castmode" "-naturalvar" "-Wextra" "-fvirtual"
  "-DPySwigIterator=RMF_PySwigIterator" "-c++"
  "-DSwigPyIterator=RMF_SwigPyIterator"
  "-DRMFEXPORT")
set(CMAKE_SWIG_OUTDIR "${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_BINARY_DIR}/include"
  "${PROJECT_SOURCE_DIR}/include" "${PROJECT_SOURCE_DIR}/swig" ${HDF5_INCLUDE_DIRS})

FILE(GLOB HEADERS "${PROJECT_SOURCE_DIR}/include/RMF/*.h" "${PROJECT_BINARY_DIR}/include/RMF/*.h")
FILE(GLOB SWIG_INCLUDES "${PROJECT_SOURCE_DIR}/swig/*.i")

SET_SOURCE_FILES_PROPERTIES(RMF.i PROPERTIES CPLUSPLUS ON)
SET(SWIG_MODULE_RMF_EXTRA_DEPS
  ${HEADERS} ${SWIG_INCLUDES}
  "${PROJECT_BINARY_DIR}/include/RMF.h"
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

FILE(GLOB swigfiles "*.i")

INSTALL(FILES ${swigfiles} DESTINATION "${CMAKE_INSTALL_SWIGDIR}")

INSTALL(TARGETS ${SWIG_MODULE_RMF_REAL_NAME} DESTINATION ${CMAKE_INSTALL_PYTHONDIR})
INSTALL(FILES "${CMAKE_BINARY_DIR}/RMF.py" DESTINATION ${CMAKE_INSTALL_PYTHONDIR})
