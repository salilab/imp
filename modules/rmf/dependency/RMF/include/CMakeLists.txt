
set(decorators "${PROJECT_BINARY_DIR}/include/RMF/physics_decorators.h"
  "${PROJECT_BINARY_DIR}/include/RMF/sequence_decorators.h"
  "${PROJECT_BINARY_DIR}/include/RMF/shape_decorators.h"
  "${PROJECT_BINARY_DIR}/include/RMF/external_decorators.h"
  "${PROJECT_BINARY_DIR}/include/RMF/feature_decorators.h"
  "${PROJECT_BINARY_DIR}/include/RMF/alias_decorators.h"
  "${PROJECT_BINARY_DIR}/include/RMF/publication_decorators.h")

add_custom_command(OUTPUT ${decorators}
  COMMAND "python" "${PROJECT_SOURCE_DIR}/scripts/make-decorators.py"
  DEPENDS "${PROJECT_SOURCE_DIR}/scripts/make-decorators.py"
  WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
  COMMENT "Making decorator headers")
add_custom_target(decorators ALL DEPENDS "${PROJECT_BINARY_DIR}/include/RMF/physics_decorators.h"
  "${PROJECT_SOURCE_DIR}/scripts/make-decorators.py")

#set(all_h "${PROJECT_BINARY_DIR}/include/RMF.h")
#add_custom_command(OUTPUT "${all_h}"
#  COMMAND python "${PROJECT_SOURCE_DIR}/scripts/make-all-header.py" RMF
#  "${PROJECT_SOURCE_DIR}/include/RMF" "${PROJECT_BINARY_DIR}/include/RMF" ">" ${all_h}
#  DEPENDS "${PROJECT_SOURCE_DIR}/scripts/make-all-header.py" decorators)
#add_custom_target(rmf_h ALL DEPENDS "${all_h}")

#INSTALL(FILES "${all_h}"  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

#set(hdf5_all_h "${PROJECT_BINARY_DIR}/include/RMF/HDF5.h")
#add_custom_command(OUTPUT "${hdf5_all_h}"
#  COMMAND python "${PROJECT_SOURCE_DIR}/scripts/make-all-header.py" RMF/HDF5
#  ${PROJECT_SOURCE_DIR}/include/RMF/HDF5 ${PROJECT_BINARY_DIR}/include/RMF/HDF5 ">" ${hdf5_all_h}
#  DEPENDS "${PROJECT_SOURCE_DIR}/scripts/make-all-header.py")
#add_custom_target(rmf_hdf5_h ALL DEPENDS "${hdf5_all_h}")

#INSTALL(FILES "${hdf5_all_h}"  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/RMF/")

INSTALL(DIRECTORY ${PROJECT_SOURCE_DIR}/include/RMF  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
INSTALL(DIRECTORY ${PROJECT_BINARY_DIR}/include/RMF  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
