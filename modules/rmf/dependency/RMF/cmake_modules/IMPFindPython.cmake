# Like cmake's FindPython but allows the user to override; should also
# work (to some degree) with older cmake
function(imp_find_python)
  if (${CMAKE_VERSION} VERSION_LESS "3.14.0")
    message(WARNING "Using old Python detection logic. Recommended to upgrade to cmake 3.14.0 or later")
    if(NOT DEFINED PYTHON_INCLUDE_DIRS)
      execute_process(COMMAND python -c "import sys; print(sys.executable)"
                      WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
		      OUTPUT_VARIABLE python_full_path
                      OUTPUT_STRIP_TRAILING_WHITESPACE)
      set(PYTHON_EXECUTABLE ${python_full_path} CACHE INTERNAL "" FORCE)
      set(PYTHON_TEST_EXECUTABLE ${python_full_path} CACHE STRING "")
      execute_process(COMMAND ${PYTHON_EXECUTABLE} -c "import sys; print('%d.%d.%d' % sys.version_info[:3])"
                      WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
                      OUTPUT_VARIABLE python_full_version
                      OUTPUT_STRIP_TRAILING_WHITESPACE)
      string(REGEX REPLACE "^([0-9])+\\.[0-9]+.*" "\\1" major
             "${python_full_version}")
      string(REGEX REPLACE "^[0-9]+\\.([0-9]+).*" "\\1" minor
             "${python_full_version}")
      string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" patch
             "${python_full_version}")
      set(PYTHON_VERSION ${python_full_version} CACHE INTERNAL "" FORCE)
      set(PYTHON_VERSION_MAJOR ${major} CACHE INTERNAL "" FORCE)
      set(PYTHON_VERSION_MINOR ${minor} CACHE INTERNAL "" FORCE)
      set(PYTHON_VERSION_PATCH ${patch} CACHE INTERNAL "" FORCE)
      message(STATUS "Python binary is " ${python_full_path} " (version " ${python_full_version} ")")

      find_package(PythonLibs ${python_full_version} EXACT REQUIRED)
      # Make sure PYTHON_INCLUDE_DIRS is in the cache so it can be
      # used elsewhere
      set(PYTHON_INCLUDE_DIRS ${PYTHON_INCLUDE_DIRS} CACHE INTERNAL "")
    endif()
    if(NOT DEFINED PYTHON_NUMPY_INCLUDE_DIR)
      execute_process(COMMAND ${PYTHON_EXECUTABLE} -c
                  "try: import numpy; print(numpy.get_include());\nexcept: pass"
                  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                  OUTPUT_VARIABLE __numpy_path
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
      find_path(PYTHON_NUMPY_INCLUDE_DIR numpy/arrayobject.h
                HINTS "${__numpy_path}" "${PYTHON_INCLUDE_PATH}"
		NO_DEFAULT_PATH)
      if(PYTHON_NUMPY_INCLUDE_DIR)
        set(PYTHON_NUMPY_FOUND 1 CACHE INTERNAL "Python numpy found")
      endif()
      include(FindPackageHandleStandardArgs)
      find_package_handle_standard_args(NumPy
                                        REQUIRED_VARS PYTHON_NUMPY_INCLUDE_DIR
                                        VERSION_VAR __numpy_version)
    endif()

  else()
    set(USE_PYTHON2 off CACHE BOOL
        "Force use of Python2 (by default Python3 is used if available)")

    if (NOT USE_PYTHON2)
      find_package(Python3 COMPONENTS Interpreter Development NumPy)
    endif()

    if(NOT USE_PYTHON2
       AND Python3_Interpreter_FOUND AND Python3_Development_FOUND)
      # Use Python 3 tools
      set(PYTHON_EXECUTABLE ${Python3_EXECUTABLE} CACHE INTERNAL "" FORCE)
      set(PYTHON_TEST_EXECUTABLE ${Python3_EXECUTABLE} CACHE STRING "")
      set(PYTHON_LIBRARIES ${Python3_LIBRARIES} CACHE INTERNAL "" FORCE)
      set(PYTHON_INCLUDE_DIRS ${Python3_INCLUDE_DIRS} CACHE INTERNAL "" FORCE)
      set(PYTHON_LIBRARY_DIRS ${Python3_LIBRARY_DIRS} CACHE INTERNAL "" FORCE)
      set(PYTHON_NUMPY_FOUND ${Python3_NumPy_FOUND} CACHE INTERNAL "" FORCE)
      set(PYTHON_NUMPY_INCLUDE_DIR ${Python3_NumPy_INCLUDE_DIRS}
          CACHE INTERNAL "" FORCE)
      set(PYTHON_VERSION ${Python3_VERSION} CACHE INTERNAL "" FORCE)
      set(PYTHON_VERSION_MAJOR ${Python3_VERSION_MAJOR} CACHE INTERNAL "" FORCE)
      set(PYTHON_VERSION_MINOR ${Python3_VERSION_MINOR} CACHE INTERNAL "" FORCE)
      set(PYTHON_VERSION_PATCH ${Python3_VERSION_PATCH} CACHE INTERNAL "" FORCE)
    else()
      find_package(Python2 COMPONENTS Interpreter Development NumPy)
      if(Python2_Interpreter_FOUND AND Python2_Development_FOUND)
        set(PYTHON_EXECUTABLE ${Python2_EXECUTABLE} CACHE INTERNAL "" FORCE)
        set(PYTHON_TEST_EXECUTABLE ${Python2_EXECUTABLE} CACHE STRING "")
        set(PYTHON_LIBRARIES ${Python2_LIBRARIES} CACHE INTERNAL "" FORCE)
        set(PYTHON_INCLUDE_DIRS ${Python2_INCLUDE_DIRS} CACHE INTERNAL "" FORCE)
        set(PYTHON_LIBRARY_DIRS ${Python2_LIBRARY_DIRS} CACHE INTERNAL "" FORCE)
        set(PYTHON_NUMPY_FOUND ${Python2_NumPy_FOUND} CACHE INTERNAL "" FORCE)
        set(PYTHON_NUMPY_INCLUDE_DIR ${Python2_NumPy_INCLUDE_DIRS}
            CACHE INTERNAL "" FORCE)
        set(PYTHON_VERSION ${Python2_VERSION} CACHE INTERNAL "" FORCE)
        set(PYTHON_VERSION_MAJOR ${Python2_VERSION_MAJOR}
            CACHE INTERNAL "" FORCE)
        set(PYTHON_VERSION_MINOR ${Python2_VERSION_MINOR}
            CACHE INTERNAL "" FORCE)
        set(PYTHON_VERSION_PATCH ${Python2_VERSION_PATCH}
            CACHE INTERNAL "" FORCE)
      else()
        message(FATAL_ERROR "Could not find a Python interpreter and matching headers/libraries. Python is required to build.")
      endif()
    endif()
  endif()
endfunction(imp_find_python)
