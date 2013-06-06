# Profiling #

# Profiling your code # {#profiling}

On linux you can use \external{http://code.google.com/p/gperftools/?redir=1 , gperftools} for code profiling. The key bits are:
- install `gperftools` (available as a pre-built package on most platforms)
- make sure debugging symbols are being included in your build by, with `g++` or `clang++` adding `-g` to your `CMAKE_CXX_FLAGS`.

- create a IMP::benchmark::Profiler in the start of the scope you want to
  profile passing the name of a file
- rebuild imp
- run your program as usual, it will create a file in the current directory
- to display your call graph run
`pprof --web <program_executable> profile_output_file` or `pprof --gv <program_executable> profile_output_file`

On a Mac, you can use the `Instruments` program that is part of the developer tools. It is pretty straight forwards to use.
