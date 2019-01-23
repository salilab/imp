find_package(MPI)

if("${MPI_CXX_FOUND}")
  message(STATUS "MPI found")
  set(MPI_CXX_FLAGS ${MPI_CXX_COMPILE_FLAGS} CACHE INTERNAL "" FORCE)
  #set(CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS} ${MPI_CXX_LINK_FLAGS})
  set(MPI_LIBRARIES ${MPI_CXX_LIBRARIES} CACHE INTERNAL "" FORCE)
  set(MPI_INCLUDE_PATH ${MPI_CXX_INCLUDE_PATH} CACHE INTERNAL "" FORCE)
  # Run tests of IMP.mpi module on 2 processors, if we found a working mpiexec
  # (note that we really should add $MPIEXEC_POSTFLAGS too, but can't right
  # now because that comes *after* python). Currently our Windows builds build
  # OK with MS-MPI, but its mpiexec doesn't work in our Wine environment
  # for some reason, so in this case we'll just test on one processor.
  if (NOT "${MPIEXEC}" MATCHES ".*NOTFOUND.*")
    set(MPI_TEST_SETUP ${MPIEXEC};${MPIEXEC_NUMPROC_FLAG};2;${MPIEXEC_PREFLAGS}
        CACHE INTERNAL "" FORCE)
  endif()
  file(WRITE "${CMAKE_BINARY_DIR}/build_info/MPI" "ok=True")
else()
  message(STATUS "MPI not found")
  file(WRITE "${CMAKE_BINARY_DIR}/build_info/MPI" "ok=False")
endif()
