if(IMP_STATIC)
  message(STATUS "RMF does not currently support static builds - skipping")
  file(WRITE "${PROJECT_BINARY_DIR}/data/build_info/RMF" "ok=False")
else()

message(STATUS "Building internal RMF")

include(ExternalProject)

set(rmf_binary_dir ${PROJECT_BINARY_DIR}/dependency/RMF.build)
set(rmf_source_dir ${PROJECT_BINARY_DIR}/dependency/RMF.source)
set(RMF_AVRO "internal" CACHE INTERNAL "Whether we are using the Avro shipped with RMF" FORCE)

ExternalProject_Add(RMF.build
  GIT_REPOSITORY "git://github.com/salilab/rmf.git"
  GIT_TAG "develop"
  SOURCE_DIR ${rmf_source_dir}
  BINARY_DIR ${rmf_binary_dir}
  #CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=
  INSTALL_COMMAND "")

set(RMF_DEPENDENCIES RMF.build)

set(RMF_INCLUDE_PATH ${rmf_source_dir}/include ${rmf_binary_dir}/include CACHE INTERNAL "" FORCE)
set(RMF_SWIG_PATH ${rmf_source_dir}/swig CACHE INTERNAL "" FORCE)
set(RMF_LINK_PATH ${rmf_binary_dir} ${rmf_binary_dir}/AvroCpp CACHE INTERNAL "" FORCE)

set(RMF_LIBRARIES RMF CACHE INTERNAL "" FORCE)

file(WRITE "${PROJECT_BINARY_DIR}/data/build_info/RMF" "ok=True
includepath=\"${RMF_INCLUDE_PATH}\"
swigpath=\"${RMF_SWIG_PATH}\"
libpath=\"${RMF_SWIG_PATH}\"
")

if(NOT DEFINED IMP_RMF_PATHS_UPDATED)
message(STATUS "Updating paths for RMF <${IMP_RMF_PATHS_UPDATED}>")
set(IMP_PYTHONPATH ${rmf_binary_dir} ${IMP_PYTHONPATH} CACHE INTERNAL "" FORCE)
set(IMP_LDPATH ${rmf_binary_dir} ${IMP_LDPATH} CACHE INTERNAL "" FORCE)
set(IMP_PATH ${rmf_binary_dir}/bin/ ${IMP_PATH} CACHE INTERNAL "" FORCE)
set(IMP_RMF_PATHS_UPDATED 1 CACHE INTERNAL "" FORCE)
endif()

file(TO_NATIVE_PATH "${PROJECT_BINARY_DIR}/dependency/RMF/" native_path)
set(ENV{PATH} "${native_path}${PATH_SEP}$ENV{PATH}")

if (${RMF_AVRO} MATCHES "internal")
  set(RMF_INCLUDE_PATH ${RMF_INCLUDE_PATH}
    ${rmf_source_dir}/AvroCpp
    CACHE INTERNAL "" FORCE)
endif()

endif()
