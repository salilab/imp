project(IMP.%(name)s)

%(tests)s

imp_get_process_exit_code("Setting up module %(name)s" status ${CMAKE_BINARY_DIR}
                          COMMAND ${PYTHON_EXECUTABLE}
                          %(tools_dir)sbuild/setup_module.py
                          %(build_dir)s --name=%(name)s
                          --datapath=${IMP_DATAPATH}
                          --defines=${IMP_%(name)s_CONFIG}:%(defines)s
                          --source=${CMAKE_SOURCE_DIR}
                          %(bin_names)s)

if(${status} EQUAL 0)
  imp_execute_process("setup_swig_wrappers %(name)s" ${CMAKE_BINARY_DIR}
    COMMAND ${PYTHON_EXECUTABLE} %(tools_dir)sbuild/setup_swig_wrappers.py
    %(build_dir)s --module=%(name)s
    --datapath=${IMP_DATAPATH}
    --source=${CMAKE_SOURCE_DIR})

  # for warning control
  add_definitions(-DIMP%(NAME)s_COMPILATION)

  set(allh_command  "${PYTHON_EXECUTABLE}" "%(tools_dir)sdev_tools/make_all_header.py" "${CMAKE_BINARY_DIR}/include/%(allh_header)s" "%(subdir)s" "${PROJECT_SOURCE_DIR}/include/" ${IMP_%(name)s_EXTRA_HEADERS})
  # for swig
  imp_execute_process("IMP.%(name)s making all header" ${PROJECT_BINARY_DIR}
                   COMMAND ${allh_command})

  add_custom_target(IMP.%(name)s-all-header
    COMMAND ${allh_command}
    DEPENDS "%(tools_dir)sdev_tools/make_all_header.py")
  set_property(TARGET "IMP.%(name)s-all-header" PROPERTY FOLDER "IMP.%(name)s")
  list(APPEND IMP_%(name)s_LIBRARY_EXTRA_DEPENDENCIES IMP.%(name)s-all-header)

  %(custom_build)s
  if(IMP_DOXYGEN_FOUND)
    # documentation
    file(GLOB headers ${CMAKE_BINARY_DIR}/include/IMP/%(name)s/*.h)
    file(GLOB docs ${CMAKE_SOURCE_DIR}/%(module_dir)sdoc/*.dox
      ${CMAKE_SOURCE_DIR}/%(module_dir)sdoc/*.md)
    file(GLOB examples ${CMAKE_BINARY_DIR}/doc/examples/%(name)s/*.py
      ${CMAKE_BINARY_DIR}/doc/examples/%(name)s/*.cpp)

    if(NOT IMP_STATIC)
      set(IMP_%(name)s_TAG_DEPENDS "${IMP.%(name)s-python}")
    endif()

    add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/doxygen/%(name)s/tags ${CMAKE_BINARY_DIR}/doxygen/%(name)s/xml/index.xml
      COMMAND mkdir -p ${CMAKE_BINARY_DIR}/doc/html
      COMMAND ln -s -f ../../include
      COMMAND ln -s -f ../../doc/examples
      COMMAND ln -s -f ../../lib
      COMMAND ${IMP_DOXYGEN_EXECUTABLE} ../../doxygen/%(name)s/Doxyfile 2>&1 /dev/null
      COMMAND ${PYTHON_EXECUTABLE} %(tools_dir)sbuild/doxygen_patch_tags.py --module=%(name)s --file=../../doxygen/%(name)s/tags
      COMMAND ${PYTHON_EXECUTABLE} %(tools_dir)sbuild/doxygen_show_warnings.py --warn=../../doxygen/%(name)s/warnings.txt
      COMMAND ${PYTHON_EXECUTABLE} %(tools_dir)sbuild/doxygen_spell_check.py xml ${CMAKE_SOURCE_DIR}/%(module_dir)stest/standards_exceptions
      DEPENDS %(tags)s ${headers} ${docs} ${examples} ${CMAKE_SOURCE_DIR}/%(module_dir)sREADME.md ${IMP_%(name)s_TAG_DEPENDS}
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/doxygen/%(name)s/
      COMMENT "Running doxygen on %(name)s")

    add_custom_target("IMP.%(name)s-doc" ALL DEPENDS ${CMAKE_BINARY_DIR}/doxygen/%(name)s/tags)
    set_property(TARGET "IMP.%(name)s-doc" PROPERTY FOLDER "IMP.%(name)s")
    set(IMP_%(name)s_DOC "IMP.%(name)s-doc" CACHE INTERNAL "" FORCE)

    if(NOT IMP_STATIC)
      list(APPEND IMP_DOC_DEPENDS "${IMP.%(name)s-python}")
    endif()
    list(REMOVE_DUPLICATES IMP_DOC_DEPENDS)
    set(IMP_DOC_DEPENDS ${IMP_DOC_DEPENDS} CACHE INTERNAL "" FORCE)
  else()
    set(IMP_%(name)s_DOC "" CACHE INTERNAL "" FORCE)
  endif(IMP_DOXYGEN_FOUND)

  list(APPEND imp_%(name)s_libs %(modules)s)
  list(APPEND imp_%(name)s_libs %(dependencies)s)
  list(REMOVE_DUPLICATES imp_%(name)s_libs)

  if(%(python_only)d EQUAL 0)
    add_custom_command(
	OUTPUT ${CMAKE_BINARY_DIR}/lib/%(subdir)s/_version_check.py
               ${CMAKE_BINARY_DIR}/src/%(name)s_config.cpp
        COMMAND ${PYTHON_EXECUTABLE} %(tools_dir)sbuild/make_module_version.py
	        --name=%(name)s --datapath=${IMP_DATAPATH}
		--source=${CMAKE_SOURCE_DIR}
        DEPENDS IMP-version
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/
        COMMENT "Building module version info")

    add_custom_target("IMP.%(name)s-version" ALL DEPENDS
                      ${CMAKE_BINARY_DIR}/lib/%(subdir)s/_version_check.py
                      ${CMAKE_BINARY_DIR}/src/%(name)s_config.cpp)
    set_property(TARGET "IMP.%(name)s-version" PROPERTY FOLDER "IMP.%(name)s")
    install(FILES "${CMAKE_BINARY_DIR}/lib/%(subdir)s/_version_check.py"
	    DESTINATION "${CMAKE_INSTALL_PYTHONDIR}/%(subdir)s/")
   endif()

  %(subdirs)s
  set(IMP_%(name)s "IMP.%(name)s" CACHE INTERNAL "" FORCE)
else()

  # make sure it is empty
  set(IMP_%(name)s_DOC "" CACHE INTERNAL "" FORCE)
  set(IMP_%(name)s_PYTHON "" CACHE INTERNAL "" FORCE)
  set(IMP_%(name)s "" CACHE INTERNAL "" FORCE)

  if(${status} EQUAL 1)
    message(%(disabled_status)s "Module IMP.%(name)s disabled")
  else()
    message(FATAL_ERROR "setup_module returned ${status}")
  endif()
  set(IMP_%(name)s_LIBRARY CACHE INTERNAL "" FORCE)
endif()
