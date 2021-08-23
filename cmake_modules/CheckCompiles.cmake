include(CheckCXXSourceCompiles)
function(check_compiles name pkgname ucpkgname includes include_dir link_libs result)
  #message(STATUS "Trying to compile ${pkgname} searching in ${include_dir} ${link_libs}")
  set(CMAKE_REQUIRED_LIBRARIES ${link_libs})
  set(CMAKE_REQUIRED_INCLUDES ${include_dir})
  set(mybody "${includes}
int main(int,char*[]) {
  ${CHECK_COMPILES_BODY}
  return 0;
}")
  set(${pkgname})
  check_cxx_source_compiles("${mybody}" "${pkgname}${name}")
  if ("${${pkgname}${name}}" MATCHES "1")
    #message(STATUS "Compiled ok")
    set(${ucpkgname}_INCLUDE_PATH ${include_dir} CACHE INTERNAL "" FORCE)
    foreach(l ${link_libs})
      set(${ucpkgname}_LIBRARIES ${${ucpkgname}_LIBRARIES} ${l} CACHE INTERNAL "" FORCE)
    endforeach()
    file(WRITE "${CMAKE_BINARY_DIR}/build_info/${pkgname}" "ok=True
includepath=\"${${pkgname}_INCLUDE_PATH}\"
swigpath=\"${${pkgname}_SWIG_PATH}\"
libpath=\"${${pkgname}_LIB_PATH}\"
")
    set(${result} "1" PARENT_SCOPE)
  else()
    set(${result} "0" PARENT_SCOPE)
  endif()
endfunction(check_compiles)
