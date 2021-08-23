include("Files.cmake")

file(GLOB_RECURSE avrocpp_sources "${PROJECT_SOURCE_DIR}/src/avrocpp/impl/*.cc")

list(REMOVE_ITEM avrocpp_sources "${PROJECT_SOURCE_DIR}/src/avrocpp/impl/avrogencpp.cc")

add_definitions("-DAVRO_DYN_LINK" "-DAVRO_SOURCE")
include_directories(BEFORE "${PROJECT_SOURCE_DIR}/src")
include_directories(BEFORE "${PROJECT_BINARY_DIR}/src")
include_directories(BEFORE "${PROJECT_SOURCE_DIR}/src/avrocpp/api/")

foreach (jf ${jsonfiles})
set(jsonfiles_expanded ${jsonfiles_expanded} ${PROJECT_SOURCE_DIR}/src/${jf})
endforeach()

if(NOT RMF_DEPRECATED_BACKENDS)
list(REMOVE_ITEM cppfiles hdf5_wrapper.cpp)
list(REMOVE_ITEM cppfiles backend/BackwardsIO.cpp)
foreach(f ${cppfiles})
if(${f} MATCHES .*deprecated.*)
message(STATUS "removing ${f}")
list(REMOVE_ITEM cppfiles ${f})
endif()
endforeach()
endif()

add_custom_command(OUTPUT "${PROJECT_BINARY_DIR}/src/generated/embed_jsons.h" "${PROJECT_BINARY_DIR}/src/generated/embed_jsons.cpp"
   COMMAND ${PYTHON_EXECUTABLE}
              "${PROJECT_SOURCE_DIR}/tools/build/make-embed.py"
              "${PROJECT_BINARY_DIR}/src/generated/" ${jsonfiles_expanded}
   DEPENDS ${jsonfiles} "${PROJECT_SOURCE_DIR}/tools/build/make-embed.py"
   COMMENT "Embedding json files" VERBATIM  )

add_custom_target(RMF-embed DEPENDS "${PROJECT_BINARY_DIR}/src/generated/embed_jsons.cpp")

set_source_files_properties("${PROJECT_BINARY_DIR}/src/generated/embed_jsons.cpp" PROPERTIES GENERATED True)

add_library(RMF-lib SHARED ${cppfiles} ${avrocpp_sources} "${PROJECT_BINARY_DIR}/src/generated/embed_jsons.cpp")
add_definitions(-DRMF_EXPORTS)
add_dependencies(RMF-lib RMF-decorators RMF-embed)
set_target_properties("RMF-lib" PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
                      OUTPUT_NAME "RMF")
set_property(TARGET "RMF-lib" PROPERTY FOLDER "RMF")
set_property(TARGET "RMF-lib" PROPERTY COMPILE_DEFINITIONS RMF_EXPORT)
set_property(TARGET "RMF-lib" PROPERTY VERSION ${RMF_SOVERSION})
set_property(TARGET "RMF-lib" PROPERTY SOVERSION ${RMF_SOVERSION})

target_link_libraries(RMF-lib
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${Boost_IOSTREAMS_LIBRARY}
  ${HDF5_LIBRARIES}
  ${Log4CXX_LIBRARY}
  )

INSTALL(TARGETS RMF-lib DESTINATION ${CMAKE_INSTALL_LIBDIR})
