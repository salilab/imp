set(wrap_py "${CMAKE_BINARY_DIR}/lib/%(subdir)s/__init__.py")
set(source "${CMAKE_BINARY_DIR}/src/%(name)s_swig/wrap.cpp"
                          "${CMAKE_BINARY_DIR}/src/%(name)s_swig/wrap.h")

add_custom_command(OUTPUT ${wrap_py}
   COMMAND  ${PYTHON_EXECUTABLE}
            "%(tools_dir)sbuild/make_python_init.py"
            --source=${CMAKE_SOURCE_DIR}
            "%(name)s"
   DEPENDS ${swigdeps} ${IMP_%(name)s_PYTHON_EXTRA_DEPENDENCIES} IMP-version
   WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
   COMMENT "Building __init__.py for %(name)s")

add_custom_target(IMP.%(name)s-python DEPENDS ${wrap_py})

set(IMP_%(name)s_PYTHON IMP.%(name)s-python ${wrap_py} %(other_pythons)s
                   CACHE INTERNAL "" FORCE)

INSTALL(FILES ${wrap_py} DESTINATION ${CMAKE_INSTALL_PYTHONDIR}/%(subdir)s)

# No C++ library target
set(IMP_%(name)s_LIBRARY "" CACHE INTERNAL "" FORCE)
