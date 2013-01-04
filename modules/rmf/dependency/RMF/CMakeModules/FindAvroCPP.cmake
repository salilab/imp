include(LibFindMacros)

#libfind_package(Log4CXX)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(AvroCPP_PKGCONF avro)

find_program(avrogencpp_PROGRAM NAMES avrogencpp)

# Include dir
find_path(AvroCPP_INCLUDE_DIR
  NAMES avro/Compiler.hh
  PATHS ${AvroCPP_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself
find_library(AvroCPP_LIBRARY
  NAMES avrocpp
  PATHS ${AvroCPP_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(AvroCPP_PROCESS_INCLUDES AvroCPP_INCLUDE_DIR)
set(AvroCPP_PROCESS_LIBS AvroCPP_LIBRARY)
libfind_process(AvroCPP)
