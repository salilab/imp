
set(decorators "${PROJECT_BINARY_DIR}/include/RMF/decorator/physics.h"
  "${PROJECT_BINARY_DIR}/include/RMF/decorator/sequence.h"
  "${PROJECT_BINARY_DIR}/include/RMF/decorator/shape.h"
  "${PROJECT_BINARY_DIR}/include/RMF/decorator/external.h"
  "${PROJECT_BINARY_DIR}/include/RMF/decorator/feature.h"
  "${PROJECT_BINARY_DIR}/include/RMF/decorator/alias.h"
  "${PROJECT_BINARY_DIR}/include/RMF/decorator/publication.h")

add_custom_command(OUTPUT ${decorators}
  COMMAND "python" "${PROJECT_SOURCE_DIR}/tools/build/make_decorators.py"
  DEPENDS "${PROJECT_SOURCE_DIR}/tools/build/make_decorators.py" "${PROJECT_SOURCE_DIR}/tools/build/_decorators.py"
  WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
  COMMENT "Making decorator headers")
add_custom_target(RMF-decorators ALL DEPENDS "${PROJECT_BINARY_DIR}/include/RMF/decorator/physics.h"
  "${PROJECT_SOURCE_DIR}/tools/build/make_decorators.py")
set_property(TARGET "RMF-decorators" PROPERTY FOLDER "RMF")

INSTALL(DIRECTORY ${PROJECT_SOURCE_DIR}/include/RMF  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
INSTALL(DIRECTORY ${PROJECT_BINARY_DIR}/include/RMF  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
