CMake configuration options {#cmake_config}
===========================

[TOC]

# Building with CMake {#cmake_building}

We use [CMake](https://cmake.org) to configure the %IMP build when
[building from source](@ref installation_compilation).

There are two different ways to configure with `cmake`; one is to run `cmake`
in a fresh directory passing some options on the command line, and the other
is to run `ccmake` and use its editor to change options. For both, assume you
are in a directory called `debug` and the %IMP source is in a directory at
`../imp`. We are using the default of makefiles for the actual building.

# Configuring with cmake command line options {#cmake_cmdline}

To configure and build as simply as possible do

    cmake ../imp
    make -j8

To make a debug build of %IMP with the `cgal` and `membrane` modules disabled
and `core` compiled in per-cpp mode, and to use
[Ninja](https://ninja-build.org/) instead of `make` as your build
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

You can use [Ninja](https://ninja-build.org/)
instead if it is available by passing `-G Ninja` to the `(c)cmake` call.
That is highly recommended when it is available.

Various aspects of %IMP build behavior can be controlled via variables. These can be set interactively using `ccmake` (eg `ccmake ../imp`) or by passing them with `-D` in a call to `cmake`. Key ones include:
- `CMAKE_BUILD_TYPE`: one of `Debug` or `Release`.
- `IMP_DISABLED_MODULES`: A colon-separated list of disabled modules.
- `IMP_MAX_CHECKS`: One of `NONE`, `USAGE`, `INTERNAL` to control what check levels will be supported. The default is `USAGE` for release builds and `INTERNAL` for debug builds (setting this to `INTERNAL` will impact performance; `NONE` is not recommended as all sanity checks will be skipped).
- `IMP_MAX_LOG`: One of `SILENT`, `PROGRESS`, `TERSE`, `VERBOSE` to control what log levels are supported.
- `IMP_PER_CPP_COMPILATION`: A colon-separated list of modules to build one .cpp at a time.
- `USE_PYTHON2`: Set to `on` to have CMake build %IMP with Python 2 (by default it will use Python 3 if available).

There also are a [variety of standard cmake options](https://gitlab.kitware.com/cmake/community/wikis/doc/cmake/Useful-Variables)
which control the build. For example:
- `CMAKE_INCLUDE_PATH` and `CMAKE_LIBRARY_PATH` control the paths CMake searches
  in to locate %IMP prerequisite libraries. If your libraries are installed in
  non-standard locations, you can set these variables to help CMake find them.
  For example, on a 32-bit RHEL5 system, which has both Boost and HDF5 in
  non-standard locations, we use

        -DCMAKE_INCLUDE_PATH="/usr/include/boost141;/usr/include/hdf518/" -DCMAKE_LIBRARY_PATH="/usr/lib/boost141;/usr/lib/hdf518"

- `CMAKE_INSTALL_PREFIX` should be set if you want to install %IMP in a
  non-standard location.

# Workarounds for common CMake issues {#cmake_issues}

## Python binary/header mismatch {#cmake_python}

In order to build %IMP Python extensions, CMake needs to find the Python header
and library files that match the `python3`, `python2` or `python` binary. If using a
recent version of CMake (3.14 or later) it should have no issues in doing so.
However, old versions of CMake might get confused if you have multiple versions
of Python installed (for example on a Mac with [Homebrew](https://brew.sh/)),
and find the headers for one version and the binary for another. This can
be worked around by explicitly telling CMake where your Python library and
headers are by setting the `PYTHON_LIBRARY` and `PYTHON_INCLUDE_DIR` CMake
variables.

For example, on a Mac with Homebrew, where `python` is Homebrew's
`/usr/local/bin/python`, old versions of CMake will often find Apple's Python
headers. This can be resolved (if you cannot upgrade CMake to 3.14 or later)
by telling CMake where the Homebrew Python headers and library
are, by addinng to your CMake invocation something like
`-DPYTHON_LIBRARY=/usr/local/opt/python@2/Frameworks/Python.framework/Versions/Current/lib/libpython2.7.dylib -DPYTHON_INCLUDE_DIR=/usr/local/opt/python@2/Frameworks/Python.framework/Versions/Current/Headers`

## CMake reports that it found a dependency but then reports failed {#cmake_compile}

For each dependency CMake will first try to find the header and library
files for that dependency, reporting success if it finds them. Next, it will
often try to build a small C or C++ test program that uses those headers
and libraries. If this fails the dependency cannot be used (and CMake will,
somewhat confusing, report that the dependency was first found and then not
found). To fix issues like this, check the CMake error log in
`CMakeFiles/CMakeError.log` to see what failed. In some cases this can be
fixed by modifying the flags passed to the C or C++ compiler. For example,
recent versions of [Protobuf](https://developers.google.com/protocol-buffers/)
fail on some systems because they require C++11 support, and this can be
fixed by adding to your CMake invocation
`-DCMAKE_CXX_FLAGS="-std=c++11"`

## Wrong version of helper binaries found {#cmake_path}

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
