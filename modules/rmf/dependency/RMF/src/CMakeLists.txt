FILE(GLOB_RECURSE SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")

configure_file (
  "${PROJECT_SOURCE_DIR}/src/internal/paths.cpp.in"
  "${PROJECT_BINARY_DIR}/src/internal/paths.cpp"
  )

add_library(RMF SHARED ${SOURCES} "${PROJECT_BINARY_DIR}/src/internal/paths.cpp"  ${decorators} ${json_headers})

target_link_libraries(RMF
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_THREAD_LIBRARY}
    avrocpp
    ${HDF5_LIBRARIES}
)

INSTALL(TARGETS RMF DESTINATION ${CMAKE_INSTALL_LIBDIR})
