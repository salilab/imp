CMake configuration options {#cmake_config}
===========================

[TOC]

# Building with CMake {#cmake_building}

We use [CMake](http://www.cmake.org) to configure the %IMP build when
[building from source](@ref installation_compilation).

There are two different ways to configure with `cmake`; one is to run `cmake`
in a fresh directory passing some options on the command line, and the other
is to run `ccmake` and use its editor to change options. For both, assume you
are in a directory called `debug` and the %IMP source is in a directory at
`../imp`. We are using the default of makefiles for the actual building.

Note that we need CMake 2.8 or later; on RHEL/CentOS systems this is provided
by the `cmake28` package in EPEL (and type `cmake28` rather than `cmake` on
the command line).

# Configuring with cmake command line options {#cmake_cmdline}

To configure and build as simply as possible do

    cmake ../imp
    make -j8

To make a debug build of %IMP with the `cgal` and `membrane` modules disabled
and `core` compiled in per-cpp mode, and to use
[Ninja](https://martine.github.io/ninja/) instead of `make` as your build
command do:

    cmake ../imp -DCMAKE_BUILD_TYPE=Debug -G Ninja -DIMP_DISABLED_MODULES=cgal:membrane -DIMP_PER_CPP_COMPILATION=core
    ninja -j8

# Configuring using ccmake {#ccmake_config}
1. Run `ccmake ../imp`
You can then look through the various options available.
2. If you want a debug build, set `CMAKE_BUILD_TYPE` to `Debug`
3. Tell cmake to configure (hit `c`) and generate (hit `g`) 
4. `make -j 8`

You can run `ccmake` after running `cmake` as above if you want, too.
Running it never hurts.

# Further configuration options {#cmake_further}

You can use `ninja` instead if it is available by passing `-G Ninja` to the `(c)cmake` call. That is highly recommended when it is available.

Various aspects of %IMP build behavior can be controlled via variables. These can be set interactively using `ccmake` (eg `ccmake ../imp`) or by passing them with `-D` in a call to `cmake`. Key ones include:
- `IMP_DISABLED_MODULES`: A colon-separated list of disabled modules.
- `IMP_MAX_CHECKS`: One of `NONE`, `USAGE`, `INTERNAL` to control what check levels will be supported.
- `IMP_MAX_LOG`: One of `SILENT`, `PROGRESS`, `TERSE`, `VERBOSE` to control what log levels are supported.
- `IMP_PER_CPP_COMPILATION`: A colon-separated list of modules to build one .cpp at a time.
- `CMAKE_BUILD_TYPE`: one of `Debug` or `Release`.

There also are a [variety of standard cmake options](http://www.cmake.org/Wiki/CMake_Useful_Variables) which control the build. In particular, if you have dependencies installed in non-standard locations, you may need to set the `CMAKE_INCLUDE_PATH` and `CMAKE_LIBRARY_PATH` variables so that cmake can find them.
If you want to install %IMP in a non-standard location you should also set the
`CMAKE_INSTALL_PREFIX` variable.
