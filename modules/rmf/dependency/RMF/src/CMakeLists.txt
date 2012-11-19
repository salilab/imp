FILE(GLOB SOURCES "src/*.cpp" "src/internal/*.cpp" "src/backend/*/*.cpp")

message(STATUS "decorators" ${decorators})
message(STATUS "json" ${json_headers})

configure_file (
  "${PROJECT_SOURCE_DIR}/src/internal/paths.cpp.in"
  "${PROJECT_BINARY_DIR}/src/internal/paths.cpp"
  )

add_library(RMF SHARED ${SOURCES} "${PROJECT_BINARY_DIR}/src/internal/paths.cpp"  ${HEADERS} ${decorators} ${json_headers})

target_link_libraries(RMF
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_THREAD_LIBRARY}
    avrocpp
    ${PROTOBUF_LIBRARY}
    ${HDF5_LIBRARIES}
)

INSTALL(TARGETS RMF DESTINATION ${CMAKE_INSTALL_LIBDIR})
