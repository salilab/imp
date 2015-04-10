execute_process(COMMAND python setup_git.py
                  RESULT_VARIABLE setup
                  OUTPUT_VARIABLE toutput
                  ERROR_VARIABLE error
                  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT ${setup} EQUAL 0)
   message(FATAL_ERROR " Failed to run setup_git.py: ${setup}; ${error}")
endif()
