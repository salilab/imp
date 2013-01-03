include(LibFindMacros)

#libfind_package(Log4CXX)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(Log4CXX_PKGCONF log4cxx)

# Include dir
find_path(Log4CXX_INCLUDE_DIR
  NAMES log4cxx/appender.h
  PATHS ${Log4CXX_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself
find_library(Log4CXX_LIBRARY
  NAMES log4cxx
  PATHS ${Log4CXX_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(Log4CXX_PROCESS_INCLUDES Log4CXX_INCLUDE_DIR)
set(Log4CXX_PROCESS_LIBS Log4CXX_LIBRARY)
libfind_process(Log4CXX)
