INSTALL(DIRECTORY ${PROJECT_SOURCE_DIR}/include/RMF  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
INSTALL(DIRECTORY ${PROJECT_BINARY_DIR}/include/RMF  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

set(decorators "${PROJECT_BINARY_DIR}/include/RMF/decorators.h")
add_custom_command(OUTPUT ${decorators}
  COMMAND "python" "${PROJECT_SOURCE_DIR}/scripts/make-decorators.py" ">" ${decorators}
  DEPENDS "${PROJECT_SOURCE_DIR}/scripts/make-decorators.py")
add_custom_target(decorators ALL DEPENDS "${decorators}")

FILE(GLOB public_headers "${PROJECT_SOURCE_DIR}/include/RMF/*.h")

set(all_h "${PROJECT_BINARY_DIR}/include/RMF.h")
add_custom_command(OUTPUT "${all_h}"
  COMMAND "${PROJECT_SOURCE_DIR}/scripts/make-all-header.py" ${public_headers} ">" ${all_h}
  DEPENDS "${PROJECT_SOURCE_DIR}/scripts/make-all-header.py")
add_custom_target(RMF_header ALL DEPENDS "${all_h}")

INSTALL(FILES "${all_h}"  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})


FILE(GLOB hdf5_public_headers "${PROJECT_SOURCE_DIR}/include/RMF/HDF5/*.h")

set(hdf5_all_h "${PROJECT_BINARY_DIR}/include/RMF/HDF5.h")
add_custom_command(OUTPUT "${hdf5_all_h}"
  COMMAND "${PROJECT_SOURCE_DIR}/scripts/make-all-header.py" ${hdf5_public_headers} ">" ${hdf5_all_h}
  DEPENDS "${PROJECT_SOURCE_DIR}/scripts/make-all-header.py")
add_custom_target(RMF_HDF5_header ALL DEPENDS "${hdf5_all_h}")

INSTALL(FILES "${hdf5_all_h}"  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/RMF/")
