include_directories("${PROJECT_BINARY_DIR}/src"
                               "${PROJECT_SOURCE_DIR}/src")

FILE(GLOB_RECURSE SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")

configure_file (
  "${PROJECT_SOURCE_DIR}/src/internal/paths.cpp.in"
  "${PROJECT_BINARY_DIR}/src/internal/paths.cpp"
  )

add_library(RMF SHARED ${SOURCES}
"${PROJECT_BINARY_DIR}/src/internal/paths.cpp" ${json_headers}
${decorators})

target_link_libraries(RMF
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_THREAD_LIBRARY}
    ${avrocpp_LIBRARY}
    ${HDF5_LIBRARIES}
)

INSTALL(TARGETS RMF DESTINATION ${CMAKE_INSTALL_LIBDIR})
