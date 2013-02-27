
FILE(GLOB BIN_SOURCES "benchmark/*.cpp")
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/bin)
foreach (cpp ${BIN_SOURCES})
   GET_FILENAME_COMPONENT(name ${cpp} NAME_WE)
   message(STATUS ${name})
   add_executable("RMF.${name}" ${cpp})
   target_link_libraries("RMF.${name}" RMF ${Boost_PROGRAM_OPTIONS_LIBRARY} )
   set(executables ${executables} "RMF.${name}")
endforeach()

add_custom_target("RMF_benchmarks" ALL DEPENDS ${executables})
