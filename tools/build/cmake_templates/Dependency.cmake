if(DEFINED IMP_NO_%(PKGNAME)s)
 %(on_failure)s
else(DEFINED IMP_NO_%(PKGNAME)s)
set(CHECK_COMPILES_BODY "%(body)s")

include(LibFindMacros)

# Use pkg-config to get hints about paths
# If there are multiple .pc files to try, try each one in turn until
# we get a result
foreach (pkg %(pkgconfigs)s)
  libfind_pkg_check_modules("%(PKGNAME)s_PKGCONF" ${pkg})
  if (NOT "${%(PKGNAME)s_PKGCONF_INCLUDE_DIRS}" STREQUAL "")
    break()
  endif()
endforeach(pkg)

# Include dir
find_path("%(PKGNAME)s_INCLUDE_DIR"
  NAMES %(headers)s
  PATHS ${%(PKGNAME)s_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself
# On Windows CMake does not search the 'lib' prefix, so manually add that;
# otherwise it will fail to find, e.g. libprotobuf.lib
foreach(lib %(libraries)s)
  if(WIN32)
    find_library("${lib}_LIBRARY"
      NAMES ${lib} lib${lib}
      PATHS ${%(PKGNAME)s_PKGCONF_LIBRARY_DIRS}
    )
  else()
    find_library("${lib}_LIBRARY"
      NAMES ${lib}
      PATHS ${%(PKGNAME)s_PKGCONF_LIBRARY_DIRS}
    )
  endif()
  list(APPEND %(PKGNAME)s_LIBRARY "${${lib}_LIBRARY}")
endforeach(lib)


# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries that this lib depends on.
set(%(PKGNAME)s_PROCESS_INCLUDES %(PKGNAME)s_INCLUDE_DIR)
set(%(PKGNAME)s_PROCESS_LIBS %(PKGNAME)s_LIBRARY)

# clear target variable
set(%(PKGNAME)s_LIBRARIES "" CACHE INTERNAL "")
libfind_process(%(PKGNAME)s)

if ("${%(PKGNAME)s_LIBRARY}" MATCHES ".*NOTFOUND.*"
    OR "${%(PKGNAME)s_INCLUDE_DIR}" MATCHES ".*NOTFOUND.*")
  %(on_failure)s
else()
  check_compiles("_found" "%(pkgname)s" "%(PKGNAME)s" "%(includes)s" "${%(PKGNAME)s_INCLUDE_DIR}" "${%(PKGNAME)s_LIBRARIES}" %(PKGNAME)s_ok_ok)
  if(${%(PKGNAME)s_ok_ok} MATCHES "1")
    message(STATUS "Found %(full_name)s")
    set(IMP_ALL_DEPENDS_VARS ${IMP_ALL_DEPENDS_VARS} "%(PKGNAME)s_INCLUDE_PATH" "%(PKGNAME)s_LIBRARIES" CACHE INTERNAL "" FORCE)
  else()
    %(on_failure)s
  endif()
endif()

%(on_setup)s
endif(DEFINED IMP_NO_%(PKGNAME)s)
