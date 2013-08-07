if(NOT DEFINED RMF_LIBRARIES)

  set(CHECK_COMPILES_BODY "")

  check_compiles("_environment" RMF RMF "#include <RMF/FileHandle.h>" "" "RMF" RMF_ok)
  if("${RMF_ok}" MATCHES "1")
    message(STATUS "Found RMF in environment")
  else("${RMF_ok}" MATCHES "1")
    include(LibFindMacros)

    # Use pkg-config to get hints about paths
    libfind_pkg_check_modules("RMF_PKGCONF" "RMF")

    # Include dir
    find_path("RMF_INCLUDE_DIR"
              NAMES RMF/FileHandle.h
              PATHS ${RMF_PKGCONF_INCLUDE_DIRS}
              )

    # Finally the library itself
    foreach(lib RMF)
      find_library("${lib}_LIBRARY"
                 NAMES ${lib}
                 PATHS ${RMF_PKGCONF_LIBRARY_DIRS}
                 )
      set("RMF_LIBRARY" ${RMF_LIBRARY} ${${lib}_LIBRARY})
    endforeach(lib)


    # Set the include dir variables and the libraries and let libfind_process do the rest.
    # NOTE: Singular variables for this library, plural for libraries this this lib depends on.
    set(RMF_PROCESS_INCLUDES RMF_INCLUDE_DIR)
    set(RMF_PROCESS_LIBS RMF_LIBRARY)
    libfind_process(RMF)

    if ("${RMF_LIBRARY}" MATCHES ".*NOTFOUND.*"
        OR "${RMF_INCLUDE_DIR}" MATCHES ".*NOTFOUND.*")
      set(RMF_INTERNAL 1 CACHE INTERNAL "" FORCE)
    else("NOTFOUND")
      check_compiles("_found" "RMF" "RMF" "#include <RMF/FileHandle.h>" "${RMF_INCLUDE_DIR}" "${RMF_LIBRARY}" RMF_ok_ok)
      if(${RMF_ok_ok} MATCHES "1")
        message(STATUS "Found RMF at ""${RMF_INCLUDE_DIR}" " " "${RMF_LIBRARY}")
      else(${RMF_ok_ok} MATCHES "1")
        set(RMF_INTERNAL 1 CACHE INTERNAL "" FORCE)
      endif(${RMF_ok_ok} MATCHES "1")
    endif("${RMF_LIBRARY}" MATCHES ".*NOTFOUND.*"
          OR "${RMF_INCLUDE_DIR}" MATCHES ".*NOTFOUND.*")
  endif("${RMF_ok}" MATCHES "1")
else(NOT DEFINED RMF_LIBRARIES)
  message(STATUS "RMF already setup")
endif(NOT DEFINED RMF_LIBRARIES)

if(DEFINED RMF_INTERNAL)
  if(IMP_STATIC)
    message(STATUS "RMF does not currently support static builds - skipping")
    file(WRITE "${CMAKE_BINARY_DIR}/data/build_info/RMF" "ok=False")
  else(IMP_STATIC)

    message(STATUS "Building internal RMF")

    set(RMF_BINARY_DIR ${CMAKE_BINARY_DIR}/src/dependency/RMF CACHE INTERNAL "" FORCE)

    add_subdirectory(${CMAKE_SOURCE_DIR}/modules/rmf/dependency/RMF_source ${RMF_BINARY_DIR})

    set(RMF_INCLUDE_PATH ${CMAKE_SOURCE_DIR}/modules/rmf/dependency/RMF_source/include ${RMF_BINARY_DIR}/include ${RMF_INCLUDE_PATH} CACHE INTERNAL "" FORCE)
    set(RMF_SWIG_PATH ${CMAKE_SOURCE_DIR}/modules/rmf/dependency/RMF_source/swig CACHE INTERNAL "" FORCE)

    set(RMF_LIBRARIES RMF-lib  CACHE INTERNAL "" FORCE)

    file(WRITE "${CMAKE_BINARY_DIR}/data/build_info/RMF" "ok=True\nincludepath=\"${RMF_INCLUDE_PATH}\"\nswigpath=\"${RMF_SWIG_PATH}\"\nlibpath=\"${RMF_SWIG_PATH}\"\n")

    if(NOT DEFINED IMP_RMF_PATHS_UPDATED)
      message(STATUS "Updating paths for RMF <${IMP_RMF_PATHS_UPDATED}>")
      set(IMP_PYTHONPATH ${CMAKE_BINARY_DIR}/src/dependency/RMF/ ${IMP_PYTHONPATH} CACHE INTERNAL "" FORCE)
      set(IMP_LDPATH ${CMAKE_BINARY_DIR}/src/dependency/RMF/ ${IMP_LDPATH} CACHE INTERNAL "" FORCE)
      set(IMP_PATH ${CMAKE_BINARY_DIR}/src/dependency/RMF/bin/ ${IMP_PATH} CACHE INTERNAL "" FORCE)
      set(IMP_RMF_PATHS_UPDATED 1 CACHE INTERNAL "" FORCE)
    endif(NOT DEFINED IMP_RMF_PATHS_UPDATED)

    file(TO_NATIVE_PATH "${CMAKE_BINARY_DIR}/src/dependency/RMF/" native_path)
    set(ENV{PATH} "${native_path}${PATH_SEP}$ENV{PATH}")

  endif(IMP_STATIC)
endif(DEFINED RMF_INTERNAL)
