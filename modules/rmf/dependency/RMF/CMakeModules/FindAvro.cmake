include(LibFindMacros)

#libfind_package(Log4CXX)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(Avro_PKGCONF avro)

find_program(avrogencpp_PROGRAM NAMES avrogencpp)

# Include dir
find_path(Avro_INCLUDE_DIR
  NAMES avro/Compiler.hh
  PATHS ${Avro_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself
find_library(Avro_LIBRARY
  NAMES avrocpp
  PATHS ${Avro_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(Avro_PROCESS_INCLUDES Avro_INCLUDE_DIR)
set(Avro_PROCESS_LIBS Avro_LIBRARY)
libfind_process(Avro)
