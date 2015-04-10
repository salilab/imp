find_package(OpenMP)
if (OPENMP_FOUND)
  set(CMAKE_REQUIRED_FLAGS ${OpenMP_CXX_FLAGS})
  set(mybody "#include <omp.h>
int main() {
#pragma omp taskwait
return 0;
}")

  check_cxx_source_compiles("${mybody}" OpenMP3)
  set(CMAKE_REQUIRED_FLAGS )
  if ("${OpenMP3}" MATCHES "1")
    message(STATUS "OpenMP 3 is supported and will be used")
  else()
    message(STATUS "OpenMP 3 not supported - will not be used")
    set(OpenMP_CXX_FLAGS "" CACHE STRING "OpenMP flags" FORCE)
    set(OPENMP_FOUND FALSE)
  endif()
endif()
