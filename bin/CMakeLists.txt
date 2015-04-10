include("Files.cmake")

include_directories(${PROJECT_SOURCE_DIR}/bin)
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_BINARY_DIR}/src)
foreach (cpp ${cppfiles})
   get_filename_component(name ${cpp} NAME_WE)
   add_executable(RMF-${name} ${cpp})
   add_dependencies(RMF-${name} RMF-decorators)
   target_link_libraries(RMF-${name} RMF-lib ${Boost_LIBRARIES} )
   set_target_properties(RMF-${name} PROPERTIES
                         RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
                         OUTPUT_NAME ${name})
   set_property(TARGET "RMF-${name}" PROPERTY FOLDER "RMF")
   install(TARGETS RMF-${name} DESTINATION ${CMAKE_INSTALL_BINDIR})
   list(APPEND bins RMF-${name})
endforeach()

set(RMF_BINS ${bins} CACHE INTERNAL "" FORCE)
