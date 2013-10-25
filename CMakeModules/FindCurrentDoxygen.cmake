imp_get_process_exit_code(ElementTree has_element_tree ${CMAKE_BINARY_DIR} COMMAND python -c "import xml.etree.ElementTree")

if(${has_element_tree})
message(STATUS "Python xml.etree not found, docs disabled.")
    set(IMP_DOXYGEN_FOUND False CACHE INTERNAL "")
    set(IMP_DOXYGEN_EXECUTABLE "${DOXYGEN_EXECUTABLE}" CACHE INTERNAL "")
else()
find_package(Doxygen QUIET)
set(needed_doxygen_version "1.8.5")
if(DOXYGEN_FOUND)
  imp_get_process_output("Doxygen version" doxygen_version ${CMAKE_BINARY_DIR} COMMAND ${DOXYGEN_EXECUTABLE} --version)
  if("${doxygen_version}" STREQUAL "${needed_doxygen_version}")
    message(STATUS "Doxygen is ${DOXYGEN_EXECUTABLE}")
    set(IMP_DOXYGEN_FOUND True CACHE INTERNAL "")
    set(IMP_DOXYGEN_EXECUTABLE ${DOXYGEN_EXECUTABLE} CACHE INTERNAL "")
  else()
    message(STATUS "Wrong doxygen version at ${DOXYGEN_EXECUTABLE}. Found ${doxygen_version}, but needed ${needed_doxygen_version}")
    set(IMP_DOXYGEN_FOUND False CACHE INTERNAL "")
    set(IMP_DOXYGEN_EXECUTABLE "not-found" CACHE INTERNAL "")
  endif()
endif(DOXYGEN_FOUND)


if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
  set(doxygen_url "http://ftp.stack.nl/pub/users/dimitri/doxygen-1.8.5.linux.bin.tar.gz" CACHE INTERNAL "")
  set(doxygen_md5 bc96e9ef776e0fd6b5d9065d92297c83)
endif()
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "MacOS")
  set(doxygen_url "http://ftp.stack.nl/pub/users/dimitri/Doxygen-1.8.5.dmg" CACHE INTERNAL "")
  set(doxygen_md5 037b5d976c147caa0617781594cb189a)
endif()

if(NOT IMP_DOXYGEN_FOUND)
  if(DEFINED doxygen_url)
    file(DOWNLOAD "http://ftp.stack.nl/" ${CMAKE_BINARY_DIR}/internet_check STATUS download_status)
    list(GET download_status 0 download_success)
    if("${download_success}" STREQUAL "0")
      set(IMP_FETCH_DOXYGEN True CACHE INTERNAL "")
    else()
      set(IMP_FETCH_DOXYGEN False CACHE INTERNAL "")
    endif()
  else()
    set(IMP_FETCH_DOXYGEN False CACHE INTERNAL "")
  endif()
endif()

if(IMP_FETCH_DOXYGEN)
    include(ExternalProject)
    message(STATUS "Will download doxygen from ${doxygen_url}")
    ExternalProject_Add( download_doxygen
            SOURCE_DIR "${CMAKE_BINARY_DIR}/tools/doxygen"
            URL ${doxygen_url}
            URL_MD5 ${doxygen_md5}
            CONFIGURE_COMMAND ${CMAKE_COMMAND} -E echo "configure"
            BUILD_COMMAND ${CMAKE_COMMAND} -E echo "build"
            INSTALL_COMMAND ${CMAKE_COMMAND} -E echo "install"
            )
    set(IMP_DOXYGEN_FETCH download_doxygen CACHE INTERNAL "")
    set(IMP_DOXYGEN_FOUND True CACHE INTERNAL "")
    set(IMP_DOXYGEN_EXECUTABLE "${CMAKE_BINARY_DIR}/tools/doxygen/bin/doxygen" CACHE INTERNAL "")
endif()
endif()