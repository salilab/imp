# Put flags in a cmake variable so we can set them before running compiler
# checks (elsewhere in the build)
set(IMP_CXX11_FLAGS "" CACHE INTERNAL "Flags to enable C++11 support" FORCE)

if (APPLE)
  execute_process(COMMAND uname -v OUTPUT_VARIABLE DARWIN_VERSION)
  string(REGEX MATCH "[0-9]+" DARWIN_VERSION ${DARWIN_VERSION})
endif()

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE
                          GCC_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
  message(STATUS "GCC version: ${GCC_VERSION}")
  if (GCC_VERSION VERSION_GREATER 6.1 OR GCC_VERSION VERSION_EQUAL 6.1)
    # gcc 6.1 and up use C++14 by default, so no explicit switch required
    # (in fact adding --std=c++11 will break compilation of packages that
    # require C++14 features, like CGAL 5).
    message(STATUS "Using g++ C++11 (or later) support")
  elseif (GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7)
    message(STATUS "Enabling g++ C++11 support")
    set(IMP_CXX11_FLAGS "--std=c++11" CACHE INTERNAL "" FORCE)
  elseif (GCC_VERSION VERSION_GREATER 4.3 OR GCC_VERSION VERSION_EQUAL 4.3)
    message(STATUS "Enabling g++ C++0x support")
    set(IMP_CXX11_FLAGS "--std=c++0x" CACHE INTERNAL "" FORCE)
  else()
    message(FATAL_ERROR "IMP requires a C++11 compiler, but this GCC appears to be too old to support C++11")
  endif()
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  execute_process(COMMAND ${CMAKE_CXX_COMPILER} --version
                  OUTPUT_VARIABLE CLANG_VERSION)
  if(CLANG_VERSION MATCHES "clang version ([0-9.]+)")
    set(CLANG_VERSION ${CMAKE_MATCH_1})
  endif()
  message(STATUS "clang version: ${CLANG_VERSION}")
  # Modern clang (6 or later) uses C++14 by default; don't force older C++11
  if(CLANG_VERSION VERSION_GREATER 6.0)
    message(STATUS "Enabling clang C++17 support")
    set(IMP_CXX11_FLAGS "--std=c++17" CACHE INTERNAL "" FORCE)
  # c++11's std::move (which boost/CGAL use) doesn't work until
  # OS X 10.9 (Darwin version 13)
  elseif(APPLE AND DARWIN_VERSION LESS 13)
    message(FATAL_ERROR "IMP requires a C++11 compiler, but this version of OS X is too old to properly support C++11")
  elseif(BOOST_VERSION LESS 105000)
    message(FATAL_ERROR "IMP requires a C++11 compiler, but this version of Boost is too old to properly support C++11")
  else()
    message(STATUS "Enabling clang C++11 support")
    set(IMP_CXX11_FLAGS "--std=c++11" CACHE INTERNAL "" FORCE)
  endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  message(STATUS "Using MSVC C++11 support")
else()
  message(STATUS "Unknown compiler, not sure what to do about C++11")
endif()

if(IMP_CXX11_FLAGS)
  add_definitions(${IMP_CXX11_FLAGS})
endif()
