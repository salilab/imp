if(IMP_STATIC)
  message(STATUS "RMF does not currently support static builds - skipping")
  file(WRITE "${CMAKE_BINARY_DIR}/data/build_info/RMF" "ok=False")
else(IMP_STATIC)

  message(STATUS "Building internal RMF")

  set(RMF_BINARY_DIR ${CMAKE_BINARY_DIR}/src/dependency/RMF CACHE INTERNAL "" FORCE)

  add_subdirectory(${CMAKE_SOURCE_DIR}/modules/rmf/dependency/RMF_source ${RMF_BINARY_DIR})

  set(RMF_INCLUDE_PATH ${CMAKE_SOURCE_DIR}/modules/rmf/dependency/RMF_source/include ${RMF_BINARY_DIR}/include ${RMF_INCLUDE_PATH} CACHE INTERNAL "" FORCE)
  list(REMOVE_DUPLICATES RMF_INCLUDE_PATH)
  set(RMF_SWIG_PATH ${CMAKE_SOURCE_DIR}/modules/rmf/dependency/RMF_source/swig CACHE INTERNAL "" FORCE)

  set(RMF_LIBRARIES RMF-lib  CACHE INTERNAL "" FORCE)

  file(WRITE "${CMAKE_BINARY_DIR}/data/build_info/RMF" "ok=True\nincludepath=\"${RMF_INCLUDE_PATH}\"\nswigpath=\"${RMF_SWIG_PATH}\"\n")
endif(IMP_STATIC)
