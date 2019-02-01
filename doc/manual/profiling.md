Profiling your code {#profiling}
===================

See also the [profiling tutorial](https://integrativemodeling.org/tutorials/profiling/)
for a walkthrough of profiling a simple %IMP application.

# Linux

On linux you can use [gperftools](https://github.com/gperftools/gperftools)
- install `gperftools` (available as a pre-built package on most platforms)
- make sure debugging symbols are being included in your build by, with `g++` or `clang++` adding `-g` to your `CMAKE_CXX_FLAGS`.

then if you are using a program that used the IMP flags support
- you can add the flag `--cpu-profiling` to profile the whole program

if not 

- either use environment variables to control profiling as in the web page above
- or add `-DCMAKE_EXE_LINKER_FLAGS=-ltcmalloc_and_profiler -DCMAKE_SHARED_LINKER_FLAGS=-ltcmalloc_and_profiler -DCMAKE_REQUIRED_FLAGS=-ltcmalloc_and_profiler` to your `cmake` invocation to add the required library to all compilations. This will activate control via %IMP flags and classes. Note that the name of the library might be different on your system. See the [gperftool docs](https://github.com/gperftools/gperftools/blob/master/README) for more details.

To profile a specific piece of code use:

`#include <IMP/benchmark/Profiler.h>`

add a Profiler before the code you want to profile:

`IMP::benchmark::Profiler pp("prof_out");`

This will produce a prof_out file that can be viewed with pprof tool.

For detection of memory leaks, run your program this way:

`env HEAPCHECK=normal your_program_exe`

it will output the leaks and also produce output file that can be viewed with pprof.

# Mac

On a Mac, you can use the `Instruments` program that is part of Xcode.
It is pretty straightforward to use and can do CPU profiling, memory profiling,
leak checking etc. It is currently located in `/Applications/Xcode.app/Contents/Applications/Instruments.app/`.
