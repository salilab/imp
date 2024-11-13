# Like cmake's FindPython but allows the user to override
function(imp_find_python)
  find_package(Python3 COMPONENTS Interpreter Development NumPy)

  if(Python3_Interpreter_FOUND AND Python3_Development_FOUND)
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
    message(FATAL_ERROR "Could not find a Python interpreter and matching headers/libraries. Python is required to build.")
  endif()
endfunction(imp_find_python)
