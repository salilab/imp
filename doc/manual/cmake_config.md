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
4. `make -j8`

You can run `ccmake` after running `cmake` as above if you want, too.
Running it never hurts.

# Further configuration options {#cmake_further}

You can use [Ninja](https://martine.github.io/ninja/)
instead if it is available by passing `-G Ninja` to the `(c)cmake` call.
That is highly recommended when it is available.

Various aspects of %IMP build behavior can be controlled via variables. These can be set interactively using `ccmake` (eg `ccmake ../imp`) or by passing them with `-D` in a call to `cmake`. Key ones include:
- `IMP_DISABLED_MODULES`: A colon-separated list of disabled modules.
- `IMP_MAX_CHECKS`: One of `NONE`, `USAGE`, `INTERNAL` to control what check levels will be supported.
- `IMP_MAX_LOG`: One of `SILENT`, `PROGRESS`, `TERSE`, `VERBOSE` to control what log levels are supported.
- `IMP_PER_CPP_COMPILATION`: A colon-separated list of modules to build one .cpp at a time.
- `CMAKE_BUILD_TYPE`: one of `Debug` or `Release`.

There also are a [variety of standard cmake options](http://www.cmake.org/Wiki/CMake_Useful_Variables) which control the build. For example:
- `CMAKE_INCLUDE_PATH` and `CMAKE_LIBRARY_PATH` control the paths CMake searches
  in to locate %IMP prerequisite libraries. If your libraries are installed in
  non-standard locations, you can set these variables to help CMake find them.
  For example, on a 32-bit RHEL5 system, which has both Boost and HDF5 in
  non-standard locations, we use

        -DCMAKE_INCLUDE_PATH="/usr/include/boost141;/usr/include/hdf518/" -DCMAKE_LIBRARY_PATH="/usr/lib/boost141;/usr/lib/hdf518"

- `CMAKE_INSTALL_PREFIX` should be set if you want to install %IMP in a
  non-standard location.

Note also that CMake searches in the system path (`PATH` environment variable)
for command line tools such as `python` and `swig`. Thus, if you have multiple
versions of tools (e.g. `/usr/bin/swig` and `/usr/local/bin/swig`) make sure
the `PATH` variable is set correctly so that the right tool is found *before*
you run CMake. You may need to make symlinks or copies to help it out if your
binaries are named oddly; for example on a RHEL5 system we need to force CMake
to use `/usr/bin/python2.6` rather than `/usr/bin/python` (which is Python 2.4,
which is too old to work with %IMP) by doing something like:

    mkdir bin
    ln -sf /usr/bin/python26 bin/python
    PATH=`pwd`/bin:$PATH
