FILE(GLOB jsons "${PROJECT_SOURCE_DIR}/data/*.json")

file(READ "src/backend/avro/Nodes.json" RMF_JSON_NODES)
file(READ "src/backend/avro/File.json" RMF_JSON_FILE)
file(READ "src/backend/avro/Data.json" RMF_JSON_DATA)

configure_file (
  "${PROJECT_SOURCE_DIR}/src/backend/avro/All.json.in"
  "${PROJECT_BINARY_DIR}/src/backend/avro/All.json"
  )

INSTALL(DIRECTORY ${PROJECT_BINARY_DIR}/data  DESTINATION ${CMAKE_INSTALL_DATADIR}/rmf)

set(json_headers)
add_custom_command(OUTPUT "${PROJECT_BINARY_DIR}/src/backend/avro/AllJSON.h"
     COMMAND "mkdir" "-p" "${PROJECT_BINARY_DIR}/src/backend/avro"
     COMMAND "${avrogen_PATH}" "--input" "${PROJECT_BINARY_DIR}/src/backend/avro/All.json"
     "--output" "${PROJECT_BINARY_DIR}/src/backend/avro/AllJSON.h" "--namespace" "RMF_internal"
     DEPENDS "${PROJECT_BINARY_DIR}/src/backend/avro/All.json" ${avrogen_DEPENDENCY} COMMENT "Creating json header")

add_custom_target("AllJSON.h" ALL DEPENDS "${PROJECT_BINARY_DIR}/src/backend/avro/All.json" )

list(APPEND json_headers "${PROJECT_BINARY_DIR}/src/backend/avro/AllJSON.h")

FILE(GLOB_RECURSE SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")

configure_file (
  "${PROJECT_SOURCE_DIR}/src/internal/paths.cpp.in"
  "${PROJECT_BINARY_DIR}/src/internal/paths.cpp"
  )

add_library(RMF SHARED ${SOURCES}
"${PROJECT_BINARY_DIR}/src/internal/paths.cpp" ${json_headers} ${decorators})

target_link_libraries(RMF
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_THREAD_LIBRARY}
    avrocpp
    ${HDF5_LIBRARIES}
)

INSTALL(TARGETS RMF DESTINATION ${CMAKE_INSTALL_LIBDIR})
