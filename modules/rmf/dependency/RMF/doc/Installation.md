Installation {#installation}
============

# Binary installation # {#binary}

The simplest way to obtain RMF is by using a pre-built binary, either
standalone or as part of [IMP](https://integrativemodeling.org).

Standalone: if you are using [Anaconda Python](https://www.anaconda.com/), install with

    conda install -c conda-forge rmf

or on a Mac with [Homebrew](https://brew.sh/), install with

    brew tap salilab/salilab; brew install rmf

IMP: Download an IMP binary (which includes RMF) from the
[IMP download page](https://integrativemodeling.org/download.html).

# Source code installation # {#source}

## Prerequisites {#installation_prereqs}

In order to build from source, you will need:

- [CMake](https://cmake.org) (2.8.12 or later; 3.14 or later is recommended)
- [Boost](https://www.boost.org) (1.53 or later; Boost.Iostreams must be built
  with its [zlib filter enabled](https://www.boost.org/doc/libs/1_67_0/libs/iostreams/doc/installation.html))
- [Python](https://www.python.org) (2.7 or later, or any version of Python 3)
- [SWIG](http://www.swig.org) (1.3.40 or later; 2.0.4 or later is needed
  if you want to use Python 3)

If you want to be able to read older format RMF files, you will also need:

- [HDF5](https://support.hdfgroup.org/HDF5/) (1.8 or later; 1.10 or 1.12
  should also work)

## Download {#installation_download}

- Use [git](https://git-scm.com/) to get the code
  directly from our [GitHub repository](https://github.com/salilab/rmf)
  with something like:

        git clone -b main https://github.com/salilab/rmf.git

  (the `main` branch tracks the most recent stable
  release; alternatively you can use `develop` to get the most recent code).

## Compilation {#installation_compilation}

Make a separate directory to keep the compiled version of RMF in (it's tidier
to keep this separate from the source code, and if you need to later you can
just delete this directory without affecting the source). Set up the build
with CMake, then finally compile it, with something like:

    mkdir build
    cd build
    cmake <path to RMF source> -DCMAKE_BUILD_TYPE=Release
    make -j8

## Testing {#installation_testing}
Once the compilation is complete, you can optionally run the test suite.
Test are run using `ctest`. A good start is to run `ctest --output-on-failure`.

Tests are labeled with the cost of the test, so to run just the cheap tests,
use `ctest -L CHEAP`.

## Installation {#installation_install}

Once everything is compiled (and optionally tested) you can install RMF
by simply running `make install`. If you opted to install in a non-standard
location, it is up to you to set up your environment variables so that RMF
can be found (you may need to set `PATH`, `PYTHONPATH`, and `LD_LIBRARY_PATH`).
