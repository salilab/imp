# Similar to INSTALL(DIRECTORY, ...) except that any symlinks in the
# hierarchy are dereferenced
function(INSTALL_DEREF src pattern dest)
  file(GLOB_RECURSE relfiles RELATIVE ${src} FOLLOW_SYMLINKS ${src}/${pattern})
  foreach(relfile ${relfiles})
    get_filename_component(subdir ${relfile} PATH)
    get_filename_component(fileext ${relfile} EXT)
    get_filename_component(filename ${relfile} NAME)
    get_filename_component(realfile "${src}/${relfile}" REALPATH)
    # If we hit a directory, this must be a symlink; recurse into the
    # linked-to directory
    if(IS_DIRECTORY ${realfile})
      install_deref("${src}/${relfile}" ${pattern} "${dest}/${relfile}")
    # Don't install .pyc (compiled Python) files or scons control files
    elseif(NOT "${fileext}" STREQUAL ".pyc"
           AND NOT "${filename}" STREQUAL "SConscript")
      install(FILES ${realfile} RENAME ${filename}
              DESTINATION "${dest}/${subdir}")
    endif()
  endforeach(relfile)
endfunction(INSTALL_DEREF)
