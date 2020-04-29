set(outinit "${CMAKE_BINARY_DIR}/lib/%(subdir)s/__init__.py")

add_custom_command(OUTPUT ${outinit}
   COMMAND  ${PYTHON_EXECUTABLE}
            "%(tools_dir)sbuild/make_python_init.py"
            --source=${CMAKE_SOURCE_DIR} %(build_dir)s
            "%(name)s"
   DEPENDS ${IMP_%(name)s_PYTHON_EXTRA_DEPENDENCIES} %(ininit)s IMP-version
   WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
   COMMENT "Building __init__.py for %(name)s")

add_custom_target(IMP.%(name)s-python ALL DEPENDS ${outinit})

set(IMP_%(name)s_PYTHON IMP.%(name)s-python ${outinit} %(other_pythons)s
                   CACHE INTERNAL "" FORCE)

INSTALL(FILES ${outinit} DESTINATION ${CMAKE_INSTALL_PYTHONDIR}/%(subdir)s)

# No C++ library target
set(IMP_%(name)s_LIBRARY "" CACHE INTERNAL "" FORCE)
