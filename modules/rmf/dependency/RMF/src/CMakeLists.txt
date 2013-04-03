include_directories("${PROJECT_BINARY_DIR}/src"
  "${PROJECT_SOURCE_DIR}/src")

FILE(GLOB sources "${PROJECT_SOURCE_DIR}/src/*.cpp" "${PROJECT_SOURCE_DIR}/src/internal/*.cpp")

include(src/backend/CMakeLists.txt)

add_library(RMF SHARED ${sources}
  ${decorators})

target_link_libraries(RMF
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${HDF5_LIBRARIES}
  ${Log4CXX_LIBRARY}
  )

INSTALL(TARGETS RMF DESTINATION ${CMAKE_INSTALL_LIBDIR})
