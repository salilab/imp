
#FILE(GLOB jsons "${PROJECT_SOURCE_DIR}/data/*.json")
set(decorators "${PROJECT_BINARY_DIR}/include/RMF/decorators.h")


#set(json_headers)
#foreach (json ${jsons})
#   GET_FILENAME_COMPONENT(name ${json} NAME_WE)
#   set(header ${PROJECT_BINARY_DIR}/include/RMF/internal/${name}.h)
#   message(STATUS ${header})
#   add_custom_command(OUTPUT ${header}
#                                        COMMAND "avrogencpp" "--input" ${json} "--output" ${header} "--namespace" "RMF_internal"
#                                        DEPENDS ${json})
#   list(APPEND json_headers ${header})
#endforeach()

add_custom_command(OUTPUT ${decorators}
                                        COMMAND "python" "${PROJECT_SOURCE_DIR}/data/make-decorators.py" ">" ${decorators}
                                        DEPENDS "${PROJECT_SOURCE_DIR}/data/make-decorators.py")
