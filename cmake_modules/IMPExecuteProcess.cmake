# execute a process and return the exit code
function(imp_get_process_exit_code name variable dir)
  set(name ${ARGV0})
  set(status ${ARGV1})
  list(REMOVE_AT ARGV 0)
  list(REMOVE_AT ARGV 0)
  list(REMOVE_AT ARGV 0)
  # Since Windows doesn't parse the #! line in Python scripts, and not all
  # Windows users opt to associate .py files with Python (and even if they
  # do, the association might point to a different Python version), run
  # Python scripts explicitly with 'python' on Windows.
  if(WIN32)
    list(LENGTH ARGV argv_len)
    if(${argv_len} GREATER 1)
      list(GET ARGV 1 prog)
      get_filename_component(prog_ext ${prog} EXT)
      if("${prog_ext}" STREQUAL ".py")
        list(INSERT ARGV 1 python)
      endif()
    endif()
  endif()
  execute_process(${ARGV}
                  RESULT_VARIABLE tstatus
                  OUTPUT_VARIABLE output
                  WORKING_DIRECTORY ${dir}
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  set(${status} ${tstatus} PARENT_SCOPE)
  if(NOT "${output}" STREQUAL "")
    message(${output})
  endif()
endfunction(imp_get_process_exit_code)

# execute a process and return the value int the passed variable
function(imp_get_process_output name variable dir)
  set(name ${ARGV0})
  set(output ${ARGV1})
  set(dir ${ARGV2})
  list(REMOVE_AT ARGV 0)
  list(REMOVE_AT ARGV 0)
  list(REMOVE_AT ARGV 0)
  if(WIN32)
    list(LENGTH ARGV argv_len)
    if(${argv_len} GREATER 1)
      list(GET ARGV 1 prog)
      get_filename_component(prog_ext ${prog} EXT)
      if("${prog_ext}" STREQUAL ".py")
        list(INSERT ARGV 1 python)
      endif()
    endif()
  endif()
  execute_process(${ARGV}
                  RESULT_VARIABLE setup
                  OUTPUT_VARIABLE toutput
                  ERROR_VARIABLE error
                  WORKING_DIRECTORY ${dir}
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  set(${output} ${toutput} PARENT_SCOPE)
  if(NOT ${setup} EQUAL 0)
    message(FATAL_ERROR " Failed to run ${ARGV}: ${setup}; ${error}")
  endif()
endfunction(imp_get_process_output)


# execute a process and check that it worked
function(imp_execute_process name dir)
  set(name ${ARGV0})
  set(dir ${ARGV1})
  list(REMOVE_AT ARGV 0)
  list(REMOVE_AT ARGV 0)
  message(STATUS "Running ${name}")
  imp_get_process_output(${name} output ${dir} ${ARGV})
  if(NOT "${output}" STREQUAL "")
    message(STATUS ${output})
  endif()
endfunction(imp_execute_process)
