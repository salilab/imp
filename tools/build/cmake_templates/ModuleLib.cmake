FILE(GLOB gensources
     "${CMAKE_BINARY_DIR}/src/%(name)s/*.cpp")

FILE(GLOB genheaders
     "${CMAKE_BINARY_DIR}/include/%(subdir)s/*.h")

include_directories(SYSTEM %(includepath)s)
link_directories(%(libpath)s)
add_definitions("-DIMP%(CPPNAME)s_EXPORTS")

include(Files.cmake)

if(NOT "${OpenMP_CXX_FLAGS}" MATCHES ".*NOTFOUND.*")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${%(NAME)s_CXX_FLAGS}")

set(headers %(headers)s)

if(DEFINED IMP_%(name)s_LIBRARY_EXTRA_SOURCES)
  set_source_files_properties(${IMP_%(name)s_LIBRARY_EXTRA_SOURCES}
                              PROPERTIES GENERATED 1)
endif()
set_source_files_properties(${CMAKE_BINARY_DIR}/src/%(name)s_config.cpp
  PROPERTIES GENERATED 1)

if(DEFINED IMP_%(name)s_IS_CUDA)
  #  FILE(GLOB IMP_%(name)s_CUDA_SOURCES "${CMAKE_SOURCE_DIR}/modules/%(name)s/src/*.cu")
  message(STATUS "Setting up cuda: " "${cudafiles}")
  FIND_PACKAGE(CUDA REQUIRED)
  INCLUDE(FindCUDA)
  LIST(APPEND CUDA_NVCC_FLAGS --compiler-options -fno-strict-aliasing -lineinfo -use_fast_math -Xptxas -dlcm=cg)
  LIST(APPEND CUDA_NVCC_FLAGS -lcufft)
  # Support for the Fermi architecture was removed in CUDA toolkit 9
  IF (CUDA_VERSION_MAJOR LESS 9)
    LIST(APPEND CUDA_NVCC_FLAGS -gencode arch=compute_20,code=sm_20)
  ENDIF()
  LIST(APPEND CUDA_NVCC_FLAGS -gencode arch=compute_30,code=sm_30)
  LIST(APPEND CUDA_NVCC_FLAGS -gencode arch=compute_35,code=sm_35)
  set(sources ${cppfiles} ${cudafiles} )
  CUDA_ADD_LIBRARY(IMP.%(name)s-lib ${gensources} ${genheaders}
    ${headers} ${sources} ${cudasources}
    ${CMAKE_BINARY_DIR}/src/%(name)s_config.cpp
    ${IMP_%(name)s_LIBRARY_EXTRA_SOURCES}
    ${IMP_LIB_TYPE}
    )
  target_link_libraries(IMP.%(name)s-lib ${CUDA_LIBRARIES} ${CUDA_curand_LIBRARY} ${CUDA_cufft_LIBRARY})
  message(STATUS "CUDA libraries: " "${CUDA_LIBRARIES}")
  message(STATUS "CUDA curand library: " "${CUDA_curand_LIBRARY}")
  message(STATUS "CUDA cufft library: " "${CUDA_cufft_LIBRARY}")
elseif(DEFINED IMP_%(name)s_IS_PER_CPP)
  set(sources ${cppfiles})
  add_library(IMP.%(name)s-lib  ${IMP_LIB_TYPE} ${gensources} ${genheaders}
              ${headers} ${sources}
              ${CMAKE_BINARY_DIR}/src/%(name)s_config.cpp
              ${IMP_%(name)s_LIBRARY_EXTRA_SOURCES}
              )
else()
  add_library(IMP.%(name)s-lib  ${IMP_LIB_TYPE} ${gensources} ${genheaders}
              ${headers} ${CMAKE_BINARY_DIR}/src/%(name)s_all.cpp
              ${CMAKE_BINARY_DIR}/src/%(name)s_config.cpp
              ${IMP_%(name)s_LIBRARY_EXTRA_SOURCES}
              )
endif()
set_target_properties(IMP.%(name)s-lib PROPERTIES
                      OUTPUT_NAME imp_%(name)s)

# Note that soversion is only set if a version is set in the *source* directory.
# This is because we have to set soversion at cmake, not build, time. All stable
# releases have a VERSION file in the source directory, so this should be OK.
IF (EXISTS ${CMAKE_SOURCE_DIR}/modules/%(name)s/VERSION)
  file(STRINGS ${CMAKE_SOURCE_DIR}/modules/%(name)s/VERSION sover
       REGEX "^[0-9]+\\.[0-9]+(\\.[0-9]+)?$")
ELSEIF(EXISTS ${CMAKE_SOURCE_DIR}/VERSION)
  file(STRINGS ${CMAKE_SOURCE_DIR}/VERSION sover
       REGEX "^[0-9]+\\.[0-9]+(\\.[0-9]+)?$")
ENDIF()
IF (NOT "${sover}" STREQUAL "")
  set_target_properties(IMP.%(name)s-lib PROPERTIES
                        VERSION ${sover} SOVERSION ${sover})
ELSE()
  set_target_properties(IMP.%(name)s-lib PROPERTIES
                        VERSION 0 SOVERSION 0)
ENDIF()

set_property(TARGET "IMP.%(name)s-lib" PROPERTY FOLDER "IMP.%(name)s")

INSTALL(TARGETS IMP.%(name)s-lib DESTINATION ${CMAKE_INSTALL_LIBDIR})

if (%(python_only)d EQUAL 0)
  add_dependencies(IMP.%(name)s-lib IMP.%(name)s-version)
endif()

if(DEFINED IMP_%(name)s_LIBRARY_EXTRA_DEPENDENCIES)
  add_dependencies(IMP.%(name)s-lib ${IMP_%(name)s_LIBRARY_EXTRA_DEPENDENCIES})
endif()

target_link_libraries(IMP.%(name)s-lib ${imp_%(name)s_libs})



set(IMP_%(name)s_LIBRARY IMP.%(name)s-lib CACHE INTERNAL "" FORCE)
