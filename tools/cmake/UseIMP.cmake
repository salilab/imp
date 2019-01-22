# UseIMP.cmake can be included in a project to build additional IMP modules.
#
# Usually this file is used with the following in CMakeLists.txt:
#
#     find_package(IMP REQUIRED)
#     include(${IMP_USE_FILE})
#
# The variables used here are defined in IMPConfig.cmake, which find_package
# should pull in.

if(NOT USE_IMP_FILE_INCLUDED)
  set(USE_IMP_FILE_INCLUDED 1)

  include(${RMF_MODULES_DIR}/IMPExecuteProcess.cmake)
  include(${RMF_MODULES_DIR}/IMPAddTests.cmake)
  include(${RMF_MODULES_DIR}/CheckCompiles.cmake)

  set(IMP_PYTHON python CACHE INTERNAL "The Python executable that IMP itself will use at runtime (the build system scripts will always use 'python')")

  if(NOT DEFINED PYTHON_INCLUDE_DIRS)
    execute_process(COMMAND ${IMP_PYTHON} -c "import sys; print(sys.executable)"
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                    OUTPUT_VARIABLE python_full_path
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${IMP_PYTHON} -c "import sys; print('%d.%d.%d' % sys.version_info[:3])"
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                    OUTPUT_VARIABLE python_full_version
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REGEX REPLACE "^([0-9]+\\.[0-9]+).*" "\\1" python_version
           ${python_full_version})
    message(STATUS "Python binary is " ${python_full_path} " (version " ${python_full_version} ")")

    FIND_PACKAGE(PythonLibs ${python_full_version} EXACT REQUIRED)
  endif()

  if(WIN32)
    set(IMP_SWIG_LIBRARIES ${PYTHON_LIBRARIES})
  endif(WIN32)

  include(GNUInstallDirs)
  # Add extra installation locations for SWIG .i files and Python code
  if(NOT DEFINED CMAKE_INSTALL_SWIGDIR)
    set(CMAKE_INSTALL_SWIGDIR "share/IMP/swig" CACHE PATH "SWIG interface files")
  endif()
  if(NOT DEFINED CMAKE_INSTALL_PYTHONDIR)
    set(CMAKE_INSTALL_PYTHONDIR "${CMAKE_INSTALL_LIBDIR}/python${python_version}/site-packages" CACHE PATH "Python modules")
  endif()

endif()
