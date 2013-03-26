if(IMP_STATIC)
  message(STATUS "RMF does not currently support static builds - skipping")
  file(WRITE "${PROJECT_BINARY_DIR}/data/build_info/RMF" "ok=False")
else()

message(STATUS "Building internal RMF")

set(RMF_BINARY_DIR ${PROJECT_BINARY_DIR}/src/dependency/RMF CACHE INTERNAL "" FORCE)

add_subdirectory(${PROJECT_SOURCE_DIR}/modules/rmf/dependency/RMF ${RMF_BINARY_DIR})

set(RMF_INCLUDE_PATH ${PROJECT_SOURCE_DIR}/modules/rmf/dependency/RMF/include ${RMF_BINARY_DIR}/include  CACHE INTERNAL "" FORCE)
set(RMF_SWIG_PATH ${PROJECT_SOURCE_DIR}/modules/rmf/dependency/RMF/swig CACHE INTERNAL "" FORCE)

set(RMF_LIBRARIES RMF  CACHE INTERNAL "" FORCE)

file(WRITE "${PROJECT_BINARY_DIR}/data/build_info/RMF" "ok=True
includepath=\"${RMF_INCLUDE_PATH}\"
swigpath=\"${RMF_SWIG_PATH}\"
libpath=\"${RMF_SWIG_PATH}\"
")

if(NOT DEFINED IMP_RMF_PATHS_UPDATED)
message(STATUS "Updating paths for RMF <${IMP_RMF_PATHS_UPDATED}>")
set(IMP_PYTHONPATH ${PROJECT_BINARY_DIR}/src/dependency/RMF/ ${IMP_PYTHONPATH} CACHE INTERNAL "" FORCE)
set(IMP_LDPATH ${PROJECT_BINARY_DIR}/src/dependency/RMF/ ${IMP_LDPATH} CACHE INTERNAL "" FORCE)
set(IMP_PATH ${PROJECT_BINARY_DIR}/src/dependency/RMF/bin/ ${IMP_PATH} CACHE INTERNAL "" FORCE)
set(IMP_RMF_PATHS_UPDATED 1 CACHE INTERNAL "" FORCE)
endif()

file(TO_NATIVE_PATH "${PROJECT_BINARY_DIR}/src/dependency/RMF/" native_path)
set(ENV{PATH} "${native_path}${PATH_SEP}$ENV{PATH}")

if (${RMF_AVRO} MATCHES "internal")
  set(RMF_INCLUDE_PATH ${RMF_INCLUDE_PATH}
    ${PROJECT_SOURCE_DIR}/modules/rmf/dependency/RMF/AvroCpp
    CACHE INTERNAL "" FORCE)
endif()

endif()
