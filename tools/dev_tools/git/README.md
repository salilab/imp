# Tools for helping with Git repositories

`setup_git.py` sets up a git repository, installing git hooks to check formatting and other common problems and bootstrapping subrepositories. To use, put a _copy_ of `bootstrap_setup_git.py` in the root of your repository and add `developer_tools` as a subrepository at `tools/dev_tools` in your main repository.

It is also recommended that you have `cmake` or your build system call that script at the start of building (so that users don't have to remember to do so themselves). To do that, add the following to the start of your `CMakeLists.txt`.

    execute_process(COMMAND python setup_git.py
                    RESULT_VARIABLE setup
                    OUTPUT_VARIABLE toutput
                    ERROR_VARIABLE error
                    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(NOT ${setup} EQUAL 0)
       message(FATAL_ERROR " Failed to run setup_git.py: ${setup}; ${error}")
    endif()

The git hooks check standards on commits and make sure there are no untracked files in the repository.