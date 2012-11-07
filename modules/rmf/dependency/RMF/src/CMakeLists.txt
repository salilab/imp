FILE(GLOB SOURCES "src/*.cpp" "src/internal/*.cpp")

message(STATUS "decorators" ${decorators})
message(STATUS "json" ${json_headers})

add_library(RMF SHARED ${SOURCES} ${HEADERS} ${decorators} ${json_headers})

target_link_libraries(RMF
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_THREAD_LIBRARY}
    ${PROTOBUF_LIBRARY}
    ${HDF5_LIBRARIES}
)

INSTALL(TARGETS RMF DESTINATION ${CMAKE_INSTALL_LIBDIR})
