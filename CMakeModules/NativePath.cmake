# Convert a cmake list (semicolon-separated) of cmake paths (directories
# separated by / characters) into a native path (e.g. on Windows directories
# are separated by \, while on Linux the list is colon-separated)
function(make_native_path cmakepath nativepath)
  set(cmakepath ${ARGV0})
  set(nativepath ${ARGV1})
  # Replace / directory separators with native separators
  file(TO_NATIVE_PATH "${cmakepath}" native)
  # If the native list separator isn't the same as cmake's, replace it
  if("${PATH_SEP}" STREQUAL ";")
    set(${nativepath} "${native}" PARENT_SCOPE)
  else()
    string(REPLACE ";" ${PATH_SEP} nativerep "${native}")
    set(${nativepath} "${nativerep}" PARENT_SCOPE)
  endif()
endfunction()
