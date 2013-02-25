
FILE(GLOB BIN_SOURCES "benchmark/*.cpp")
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/bin)
foreach (cpp ${BIN_SOURCES})
   GET_FILENAME_COMPONENT(name ${cpp} NAME_WE)
   message(STATUS ${name})
   add_executable(${name} ${cpp})
   target_link_libraries(${name} RMF ${Boost_PROGRAM_OPTIONS_LIBRARY} )
   set(executables ${executables} ${name})
endforeach()

add_custom_target("RMF_benchmarks" ALL DEPENDS ${executables})
