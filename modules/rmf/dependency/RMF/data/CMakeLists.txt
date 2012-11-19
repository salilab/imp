
FILE(GLOB jsons "${PROJECT_SOURCE_DIR}/data/*.json")
set(decorators "${PROJECT_BINARY_DIR}/include/RMF/decorators.h")

file(READ "data/Nodes.json" RMF_JSON_NODES)
file(READ "data/File.json" RMF_JSON_FILE)
file(READ "data/Data.json" RMF_JSON_DATA)
file(READ "data/Frames.json" RMF_JSON_FRAMES)

configure_file (
  "${PROJECT_SOURCE_DIR}/data/All.json.in"
  "${PROJECT_BINARY_DIR}/data/All.json"
  )

INSTALL(DIRECTORY ${PROJECT_BINARY_DIR}/data  DESTINATION ${CMAKE_INSTALL_DATADIR}/rmf)

set(json_headers)
add_custom_command(OUTPUT "${PROJECT_BINARY_DIR}/src/backend/avro/AllJSON.h"
     COMMAND "mkdir" "-p" "${PROJECT_BINARY_DIR}/src/backend/avro"
     COMMAND "${PROJECT_BINARY_DIR}/AvroCpp/avrogencpp" "--input" "${PROJECT_BINARY_DIR}/data/All.json"
     "--output" "${PROJECT_BINARY_DIR}/src/backend/avro/AllJSON.h" "--namespace" "RMF_internal"
     DEPENDS "${PROJECT_BINARY_DIR}/data/All.json"
     "${PROJECT_BINARY_DIR}/AvroCpp/avrogencpp"  COMMENT "Creating json header")

add_custom_target("AllJSON.h" ALL DEPENDS "${PROJECT_BINARY_DIR}/data/All.json")

list(APPEND json_headers "${PROJECT_BINARY_DIR}/src/backend/avro/AllJSON.h")

#foreach (json ${jsons})
#   GET_FILENAME_COMPONENT(name ${json} NAME_WE)
#   set(header ${PROJECT_BINARY_DIR}/include/RMF/internal/${name}.h)
#   add_custom_command(OUTPUT ${header}
#     COMMAND "mkdir" "-p" "${PROJECT_BINARY_DIR}/include/RMF/internal"
#     COMMAND "${PROJECT_BINARY_DIR}/AvroCpp/avrogencpp" "--input" ${json} "--output" ${header} "--namespace" "RMF_internal"
#     DEPENDS ${json} COMMENT "Creating ${header} from ${json}")
#   add_custom_target(${name} ALL DEPENDS ${json})
#   list(APPEND json_headers ${header})
#endforeach()

add_custom_command(OUTPUT ${decorators}
  COMMAND "python" "${PROJECT_SOURCE_DIR}/data/make-decorators.py" ">" ${decorators}
  DEPENDS "${PROJECT_SOURCE_DIR}/data/make-decorators.py")
