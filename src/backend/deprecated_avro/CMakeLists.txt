# AllJSON.h

file(READ "backend/avro/Nodes.json" RMF_JSON_NODES)
file(READ "backend/avro/File.json" RMF_JSON_FILE)
file(READ "backend/avro/Data.json" RMF_JSON_DATA)
file(READ "backend/avro/Frame.json" RMF_JSON_FRAME)

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
   COMMAND python "${PROJECT_SOURCE_DIR}/tools/build/make-embed.py"
              "${PROJECT_BINARY_DIR}/src/backend/avro/"
              "${PROJECT_SOURCE_DIR}/src/backend/avro/File.json" "avro_backend"
   DEPENDS "${PROJECT_SOURCE_DIR}/src/backend/avro/File.json" "${PROJECT_SOURCE_DIR}/tools/build/make-embed.py"
   COMMENT "Data wrapper for File.json"  )

add_custom_command(OUTPUT "${PROJECT_BINARY_DIR}/src/backend/avro/Data.json.h" "${PROJECT_BINARY_DIR}/src/backend/avro/Data.json.cpp"
   COMMAND python "${PROJECT_SOURCE_DIR}/tools/build/make-embed.py"
              "${PROJECT_BINARY_DIR}/src/backend/avro/"
              "${PROJECT_SOURCE_DIR}/src/backend/avro/Data.json" "avro_backend"
   DEPENDS "${PROJECT_SOURCE_DIR}/src/backend/avro/Data.json" "${PROJECT_SOURCE_DIR}/tools/build/make-embed.py"
   COMMENT "Data wrapper for Data.json"  )

add_custom_command(OUTPUT "${PROJECT_BINARY_DIR}/src/backend/avro/Nodes.json.h" "${PROJECT_BINARY_DIR}/src/backend/avro/Nodes.json.cpp"
   COMMAND python "${PROJECT_SOURCE_DIR}/tools/build/make-embed.py"
              "${PROJECT_BINARY_DIR}/src/backend/avro/"
              "${PROJECT_SOURCE_DIR}/src/backend/avro/Nodes.json" "avro_backend"
   DEPENDS "${PROJECT_SOURCE_DIR}/src/backend/avro/Nodes.json" "${PROJECT_SOURCE_DIR}/tools/build/make-embed.py"
   COMMENT "Nodes wrapper for Nodes.json"  )

add_custom_command(OUTPUT "${PROJECT_BINARY_DIR}/src/backend/avro/Frame.json.h" "${PROJECT_BINARY_DIR}/src/backend/avro/Frame.json.cpp"
   COMMAND python "${PROJECT_SOURCE_DIR}/tools/build/make-embed.py"
              "${PROJECT_BINARY_DIR}/src/backend/avro/"
              "${PROJECT_SOURCE_DIR}/src/backend/avro/Frame.json" "avro_backend"
   DEPENDS "${PROJECT_SOURCE_DIR}/src/backend/avro/Frame.json" "${PROJECT_SOURCE_DIR}/tools/build/make-embed.py"
   COMMENT "Frame wrapper for Frame.json"  )

add_custom_command(OUTPUT "${PROJECT_BINARY_DIR}/src/backend/avro/All.json.h" "${PROJECT_BINARY_DIR}/src/backend/avro/All.json.cpp"
   COMMAND python "${PROJECT_SOURCE_DIR}/tools/build/make-embed.py"
              "${PROJECT_BINARY_DIR}/src/backend/avro/"
              "${PROJECT_BINARY_DIR}/src/backend/avro/All.json" "avro_backend"
   DEPENDS "${PROJECT_BINARY_DIR}/src/backend/avro/All.json" "${PROJECT_SOURCE_DIR}/tools/build/make-embed.py"
   COMMENT "All wrapper for All.json"  )

set(datas "${PROJECT_BINARY_DIR}/src/backend/avro/All.json.h" "${PROJECT_BINARY_DIR}/src/backend/avro/All.json.cpp"
    "${PROJECT_BINARY_DIR}/src/backend/avro/Frame.json.h" "${PROJECT_BINARY_DIR}/src/backend/avro/Frame.json.cpp"
    "${PROJECT_BINARY_DIR}/src/backend/avro/Nodes.json.h" "${PROJECT_BINARY_DIR}/src/backend/avro/Nodes.json.cpp"
    "${PROJECT_BINARY_DIR}/src/backend/avro/Data.json.h" "${PROJECT_BINARY_DIR}/src/backend/avro/Data.json.cpp"
    "${PROJECT_BINARY_DIR}/src/backend/avro/File.json.h" "${PROJECT_BINARY_DIR}/src/backend/avro/File.json.cpp" "${PROJECT_SOURCE_DIR}/tools/build/make-embed.py")

list(APPEND cppfiles ${avro_sources}
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
