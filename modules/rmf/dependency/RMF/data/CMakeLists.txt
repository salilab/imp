FILE(GLOB jsons "${PROJECT_SOURCE_DIR}/data/*.json")

file(READ "data/Nodes.json" RMF_JSON_NODES)
file(READ "data/File.json" RMF_JSON_FILE)
file(READ "data/Data.json" RMF_JSON_DATA)

configure_file (
  "${PROJECT_SOURCE_DIR}/data/All.json.in"
  "${PROJECT_BINARY_DIR}/data/All.json"
  )

INSTALL(DIRECTORY ${PROJECT_BINARY_DIR}/data  DESTINATION ${CMAKE_INSTALL_DATADIR}/rmf)

set(json_headers)
add_custom_command(OUTPUT "${PROJECT_BINARY_DIR}/src/backend/avro/AllJSON.h"
     COMMAND "mkdir" "-p" "${PROJECT_BINARY_DIR}/src/backend/avro"
     COMMAND "${avrogencpp_PROGRAM}" "--input" "${PROJECT_BINARY_DIR}/data/All.json"
     "--output" "${PROJECT_BINARY_DIR}/src/backend/avro/AllJSON.h" "--namespace" "RMF_internal"
     DEPENDS "${PROJECT_BINARY_DIR}/data/All.json" ${avrogen_DEPENDENCY} COMMENT "Creating json header")

add_custom_target("AllJSON.h" ALL DEPENDS "${PROJECT_BINARY_DIR}/data/All.json" )

list(APPEND json_headers "${PROJECT_BINARY_DIR}/src/backend/avro/AllJSON.h")


configure_file (
  "data/json.dox.in"
  "${PROJECT_BINARY_DIR}/doc/json.dox"
  )
