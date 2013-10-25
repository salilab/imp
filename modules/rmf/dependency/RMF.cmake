
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
        message(STATUS "Found RMF at ${RMF_INCLUDE_DIR} ${RMF_LIBRARY}")
      else(${RMF_ok_ok} MATCHES "1")
        set(RMF_INTERNAL 1 CACHE INTERNAL "" FORCE)
      endif(${RMF_ok_ok} MATCHES "1")
    endif("${RMF_LIBRARY}" MATCHES ".*NOTFOUND.*"
          OR "${RMF_INCLUDE_DIR}" MATCHES ".*NOTFOUND.*")
  endif("${RMF_ok}" MATCHES "1")

if(DEFINED RMF_INTERNAL AND NOT DEFINED RMF_LIBRARIES)
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

    file(WRITE "${CMAKE_BINARY_DIR}/data/build_info/RMF" "ok=True\nincludepath=\"${RMF_INCLUDE_PATH}\"\nswigpath=\"${RMF_SWIG_PATH}\"\n")
  endif(IMP_STATIC)
endif(DEFINED RMF_INTERNAL AND NOT DEFINED RMF_LIBRARIES)
