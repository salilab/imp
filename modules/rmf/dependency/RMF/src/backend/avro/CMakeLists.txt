# AllJSON.h

file(READ "src/backend/avro/Nodes.json" RMF_JSON_NODES)
file(READ "src/backend/avro/File.json" RMF_JSON_FILE)
file(READ "src/backend/avro/Data.json" RMF_JSON_DATA)
file(READ "src/backend/avro/Frame.json" RMF_JSON_FRAME)

configure_file (
  "${PROJECT_SOURCE_DIR}/src/backend/avro/All.json.in"
  "${PROJECT_BINARY_DIR}/src/backend/avro/All.json"
  )

#add_custom_command(OUTPUT "${PROJECT_BINARY_DIR}/src/backend/avro/FrameJSON.h"
#     COMMAND "mkdir" "-p" "${PROJECT_BINARY_DIR}/src/backend/avro"
#     COMMAND "${avrogencpp_PROGRAM}" "--input" #"${PROJECT_SOURCE_DIR}/src/backend/avro/Frame.json"
#     "--output" "${PROJECT_BINARY_DIR}/src/backend/avro/FrameJSON.h" "--namespace" #"RMF_avro_backend"
#     DEPENDS "${PROJECT_SOURCE_DIR}/src/backend/avro/Frame.json" ${avrogen_DEPENDENCY} #COMMENT "Creating json header")

#add_custom_target("FrameJSON.h" ALL DEPENDS #"${PROJECT_SOURCE_DIR}/src/backend/avro/Frame.json" )


#add_custom_command(OUTPUT "${PROJECT_BINARY_DIR}/src/backend/avro/AllJSON.h"
#     COMMAND "mkdir" "-p" "${PROJECT_BINARY_DIR}/src/backend/avro"
#     COMMAND "${avrogencpp_PROGRAM}" "--input" #"${PROJECT_BINARY_DIR}/src/backend/avro/All.json"
#     "--output" "${PROJECT_BINARY_DIR}/src/backend/avro/AllJSON.h" "--namespace" #"RMF_avro_backend"
#     DEPENDS "${PROJECT_BINARY_DIR}/src/backend/avro/All.json" ${avrogen_DEPENDENCY} #COMMENT "Creating json header")

#add_custom_target("AllJSON.h" ALL DEPENDS #"${PROJECT_BINARY_DIR}/src/backend/avro/All.json" )

# Data includes
add_custom_command(OUTPUT "${PROJECT_BINARY_DIR}/src/backend/avro/File.json.h" "${PROJECT_BINARY_DIR}/src/backend/avro/File.json.cpp"
   COMMAND python "${PROJECT_SOURCE_DIR}/scripts/make-embed.py"
              "${PROJECT_BINARY_DIR}/src/backend/avro/"
              "${PROJECT_SOURCE_DIR}/src/backend/avro/File.json" "avro_backend"
   DEPENDS "${PROJECT_SOURCE_DIR}/src/backend/avro/File.json" "${PROJECT_SOURCE_DIR}/scripts/make-embed.py"
   COMMENT "Data wrapper for File.json"  )

add_custom_target("File.data" ALL DEPENDS "${PROJECT_BINARY_DIR}/src/backend/avro/File.json.h" "${PROJECT_BINARY_DIR}/src/backend/avro/File.json.cpp" "${PROJECT_SOURCE_DIR}/scripts/make-embed.py"  )

add_custom_command(OUTPUT "${PROJECT_BINARY_DIR}/src/backend/avro/Data.json.h" "${PROJECT_BINARY_DIR}/src/backend/avro/Data.json.cpp"
   COMMAND python "${PROJECT_SOURCE_DIR}/scripts/make-embed.py"
              "${PROJECT_BINARY_DIR}/src/backend/avro/"
              "${PROJECT_SOURCE_DIR}/src/backend/avro/Data.json" "avro_backend"
   DEPENDS "${PROJECT_SOURCE_DIR}/src/backend/avro/Data.json" "${PROJECT_SOURCE_DIR}/scripts/make-embed.py"
   COMMENT "Data wrapper for Data.json"  )

add_custom_target("Data.data" ALL DEPENDS "${PROJECT_BINARY_DIR}/src/backend/avro/Data.json.h" "${PROJECT_BINARY_DIR}/src/backend/avro/Data.json.cpp" "${PROJECT_SOURCE_DIR}/scripts/make-embed.py"  )

add_custom_command(OUTPUT "${PROJECT_BINARY_DIR}/src/backend/avro/Nodes.json.h" "${PROJECT_BINARY_DIR}/src/backend/avro/Nodes.json.cpp"
   COMMAND python "${PROJECT_SOURCE_DIR}/scripts/make-embed.py"
              "${PROJECT_BINARY_DIR}/src/backend/avro/"
              "${PROJECT_SOURCE_DIR}/src/backend/avro/Nodes.json" "avro_backend"
   DEPENDS "${PROJECT_SOURCE_DIR}/src/backend/avro/Nodes.json" "${PROJECT_SOURCE_DIR}/scripts/make-embed.py"
   COMMENT "Nodes wrapper for Nodes.json"  )

add_custom_target("Nodes.data" ALL DEPENDS "${PROJECT_BINARY_DIR}/src/backend/avro/Nodes.json.h" "${PROJECT_BINARY_DIR}/src/backend/avro/Nodes.json.cpp" "${PROJECT_SOURCE_DIR}/scripts/make-embed.py"  )

add_custom_command(OUTPUT "${PROJECT_BINARY_DIR}/src/backend/avro/Frame.json.h" "${PROJECT_BINARY_DIR}/src/backend/avro/Frame.json.cpp"
   COMMAND python "${PROJECT_SOURCE_DIR}/scripts/make-embed.py"
              "${PROJECT_BINARY_DIR}/src/backend/avro/"
              "${PROJECT_SOURCE_DIR}/src/backend/avro/Frame.json" "avro_backend"
   DEPENDS "${PROJECT_SOURCE_DIR}/src/backend/avro/Frame.json" "${PROJECT_SOURCE_DIR}/scripts/make-embed.py"
   COMMENT "Frame wrapper for Frame.json"  )

add_custom_target("Frame.data" ALL DEPENDS "${PROJECT_BINARY_DIR}/src/backend/avro/Frame.json.h" "${PROJECT_BINARY_DIR}/src/backend/avro/Frame.json.cpp" "${PROJECT_SOURCE_DIR}/scripts/make-embed.py"  )


add_custom_command(OUTPUT "${PROJECT_BINARY_DIR}/src/backend/avro/All.json.h" "${PROJECT_BINARY_DIR}/src/backend/avro/All.json.cpp"
   COMMAND python "${PROJECT_SOURCE_DIR}/scripts/make-embed.py"
              "${PROJECT_BINARY_DIR}/src/backend/avro/"
              "${PROJECT_BINARY_DIR}/src/backend/avro/All.json" "avro_backend"
   DEPENDS "${PROJECT_BINARY_DIR}/src/backend/avro/All.json" "${PROJECT_SOURCE_DIR}/scripts/make-embed.py"
   COMMENT "All wrapper for All.json"  )

add_custom_target("All.data" ALL DEPENDS "${PROJECT_BINARY_DIR}/src/backend/avro/All.json.h" "${PROJECT_BINARY_DIR}/src/backend/avro/All.json.cpp" "${PROJECT_SOURCE_DIR}/scripts/make-embed.py"  )


include("${PROJECT_SOURCE_DIR}/src/backend/avro/Files.cmake")

file(GLOB_RECURSE avrocpp_sources "${PROJECT_SOURCE_DIR}/src/backend/avro/AvroCpp/impl/*.cc")

list(REMOVE_ITEM avrocpp_sources "${PROJECT_SOURCE_DIR}/src/backend/avro/AvroCpp/impl/avrogencpp.cc")

add_definitions("-DAVRO_DYN_LINK" "-DAVRO_SOURCE")
include_directories("${PROJECT_SOURCE_DIR}/src/backend/avro/AvroCpp/api")

list(APPEND sources ${avro_sources} ${avrocpp_sources}
"${PROJECT_BINARY_DIR}/src/backend/avro/All.json.cpp"
"${PROJECT_BINARY_DIR}/src/backend/avro/File.json.cpp"
"${PROJECT_BINARY_DIR}/src/backend/avro/Frame.json.cpp"
"${PROJECT_BINARY_DIR}/src/backend/avro/Nodes.json.cpp"
"${PROJECT_BINARY_DIR}/src/backend/avro/Data.json.cpp"
"${PROJECT_BINARY_DIR}/src/backend/avro/All.json.h"
"${PROJECT_BINARY_DIR}/src/backend/avro/File.json.h"
"${PROJECT_BINARY_DIR}/src/backend/avro/Frame.json.h"
"${PROJECT_BINARY_DIR}/src/backend/avro/Nodes.json.h"
"${PROJECT_BINARY_DIR}/src/backend/avro/Data.json.h")
