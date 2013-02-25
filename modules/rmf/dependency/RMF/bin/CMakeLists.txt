
FILE(GLOB BIN_SOURCES "*.cpp")
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/bin)
foreach (cpp ${BIN_SOURCES})
   GET_FILENAME_COMPONENT(name ${cpp} NAME_WE)
   add_executable(${name} ${cpp})
   target_link_libraries(${name} RMF ${Boost_LIBRARIES} )
   INSTALL(TARGETS ${name} DESTINATION ${CMAKE_INSTALL_BINDIR})
   set(executables ${executables} ${name})
endforeach()

add_custom_target("RMF_bins" ALL DEPENDS ${executables})
