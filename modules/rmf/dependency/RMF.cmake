set(IMP_USE_SYSTEM_RMF off CACHE BOOL "Use an external (system) copy of RMF, rather than that bundled with IMP.")

if(IMP_USE_SYSTEM_RMF)
  message(STATUS "Using system (external) RMF")
  find_package(RMF 1.3 REQUIRED)

  set(RMF_LIBRARIES "${RMF_LIBRARY}" CACHE INTERNAL "" FORCE)

  file(WRITE "${CMAKE_BINARY_DIR}/build_info/RMF" "ok=True\nincludepath=\"${RMF_INCLUDE_PATH}\"\nswigpath=\"${RMF_SWIG_PATH}\"\n")
else(IMP_USE_SYSTEM_RMF)

if(IMP_STATIC)
  message(STATUS "RMF does not currently support static builds - skipping")
  file(WRITE "${CMAKE_BINARY_DIR}/build_info/RMF" "ok=False")
else(IMP_STATIC)

  message(STATUS "Building internal RMF")

  set(RMF_BINARY_DIR ${CMAKE_BINARY_DIR}/src/dependency/RMF CACHE INTERNAL "" FORCE)

  add_subdirectory(${CMAKE_SOURCE_DIR}/modules/rmf/dependency/RMF ${RMF_BINARY_DIR})

  set(RMF_INCLUDE_PATH ${CMAKE_SOURCE_DIR}/modules/rmf/dependency/RMF/include ${RMF_BINARY_DIR}/include ${RMF_INCLUDE_PATH} CACHE INTERNAL "" FORCE)
  list(REMOVE_DUPLICATES RMF_INCLUDE_PATH)
  set(RMF_SWIG_PATH ${CMAKE_SOURCE_DIR}/modules/rmf/dependency/RMF/swig CACHE INTERNAL "" FORCE)

  set(RMF_LIBRARIES RMF-lib  CACHE INTERNAL "" FORCE)

  file(WRITE "${CMAKE_BINARY_DIR}/build_info/RMF" "ok=True\nincludepath=\"${RMF_INCLUDE_PATH}\"\nswigpath=\"${RMF_SWIG_PATH}\"\n")
endif(IMP_STATIC)

endif(IMP_USE_SYSTEM_RMF)
