Installation {#installation}
============

[TOC]

# Binary installation {#installation_binary}

Binary installation is strongly recommended for new users of %IMP. It is
much faster than building from source code, requires a smaller download,
and all the necessary prerequisites are handled for you automatically.

We recommend you use a stable release. These are available for
Windows, Mac and Linux from our [download page](http://integrativemodeling.org/download.html#stable).

Binaries are [also available for our latest nightly builds](http://integrativemodeling.org/download.html#develop). If you do decide to use a nightly build,
please check out the [nightly builds results page](http://integrativemodeling.org/nightly/results/)
to see if the code is currently stable enough for your purposes.

# Source code installation {#installation_source}

## Prerequisites {#installation_prereqs}

In order to build %IMP from source, you will need:

- [CMake](http://www.cmake.org) (2.8 or later)
- [Boost](http://www.boost.org) (1.40 or later)
- [HDF5](http://www.hdfgroup.org/HDF5/) (1.8 or later)
- [Python](http://www.python.org) (2.6 or later, or any version of Python 3)
- [SWIG](http://www.swig.org) (1.3.40 or later; 2.0.4 or later is needed
  if you want to use Python 3)

The following prerequisites are _optional_; without them some parts of %IMP
will not build, and some will not function optimally.

- [Doxygen](http://www.doxygen.org/) (only exactly version 1.8.6 is supported)
  and [Graphviz](http://www.graphviz.org/): required for building
  documentation.
- [Modeller](\ref modeller): needed to use the IMP.modeller module.
- [CGAL](\ref CGAL): enables faster geometric operations, such as
  nonbonded lists.
- [Google perf tools](\ref perf): needed only for profiling %IMP code.
- [ANN](\ref ANN): certain data structures will be faster if it is available.
- [GSL](\ref GSL): needed to use the IMP.gsl module.
- [OpenCV](\ref OpenCV): needed to use the IMP.em2d module or the
  [idock](@ref idock_pcsk9) and [emagefit](@ref emagefit_3sfd) command
  line tools.
- [FFTW](http://www.fftw.org): need to use the IMP.em2d or IMP.multifit modules
  or the [multifit](@ref multifit_3sfd) command line tool.
- [libTAU](http://integrativemodeling.org/libTAU.html): needed to use the
  IMP.cnmultifit module or the [cnmultifit](@ref cnmultifit_groel) command
  line tool.
- An [MPI](@ref IMP::mpi) library is needed to use the IMP.mpi module.
- The [numpy, scipy](http://www.scipy.org/scipylib/download.html),
  [scikit-learn](http://scikit-learn.org/stable/install.html),
  [matplotlib](http://matplotlib.org/downloads.html) and
  [biopython](http://biopython.org/wiki/Download) Python libraries are also
  recommended.
- [Chimera](https://www.cgl.ucsf.edu/chimera/download.html) is recommended
  for visualization of results.

### Getting prerequisites on Linux {#installation_prereqs_linux}
All of the prerequisites should be available as pre-built packages for
your Linux distribution of choice.

### Getting prerequisites on a Mac {#installation_prereqs_mac}

Mac users must first install Xcode (previously known as Developer Tools)
which is not installed by default with OS X, but is available from the App store
(or from the Mac OS install DVD for old versions of Mac OS). They will also
need the Xcode command line tools (install by going to Xcode Preferences, then
Downloads, then Components, and select "Command Line Tools").

Then Mac users should use one of the available collections of Unix tools,
such as
- [Homebrew](http://brew.sh) (_recommended_) Once you installed `homebrew`
  do

        brew tap homebrew/science
        brew tap salilab/salilab
        brew install boost gmp google-perftools cgal graphviz gsl cmake doxygen hdf5 swig fftw mpfr libtau

  to install everything %IMP finds useful (or that you will want for installing various useful Python libs that %IMP finds useful). On older Macs, you may also need to `brew install git` if you want to use git (newer Macs include git).
- [Macports](http://www.macports.org/) If you use MacPorts, you must add `/opt/local/bin` to your path (either by modifying your shell's
  config file or by making an `environment.plist` file) and then do

        sudo port install boost cmake swig-python

  to install the needed libraries and tools. When installing HDF5 with MacPorts, be sure to install `hdf5-18`
  (version 1.8), rather than the older `hdf5` (version 1.6.9).
- or [Fink](http://www.finkproject.org/) (not supported)

### Getting prerequisites on Windows {#installation_prereqs_windows}

We recommend Linux or Mac for developing with %IMP, as obtaining the
prerequisites on Windows is much more involved. However, if you really want
to build on Windows, see the
[building from source code on Windows](@ref install_windows) page for the
procedure we use.


## Download {#installation_download}

- Download the source code tarball from [our download page](http://integrativemodeling.org/download.html#source), then extract it with something like:

        tar -xvzf ../imp-<version>.tar.gz

- Alternatively you can use [git](http://git-scm.com/) to get the code
  directly from our [GitHub repository](https://github.com/salilab/imp)
  with something like:

        git clone -b master https://github.com/salilab/imp.git
        (cd imp && ./setup_git.py)

  (the `master` branch tracks the most recent stable
  release; alternatively you can use `develop` to get the most recent code,
  but please check out the [nightly builds results page](http://integrativemodeling.org/nightly/results/)
  to see if the code is currently stable enough for your purposes).

## Compilation {#installation_compilation}

Make a separate directory to keep the compiled version of %IMP in (it's tidier
to keep this separate from the source code, and if you need to later you can
just delete this directory without affecting the source). Set up the build
with [CMake](@ref cmake_config), then finally compile it, with something
like:

    mkdir imp_release
    cd imp_release
    cmake <path to IMP source>
    make -j8

There are a number of ways in which %IMP can be configured.
See [the configuration options page](@ref cmake_config) for more details.

## Testing {#installation_testing}
Once the compilation is complete, you can optionally run the test suite.
Test are run using `ctest`. A good start is to run `ctest --output-on-failure`.

Tests are labeled with the module name and the type and cost of the test, so to run just the expensive tests in the `atom` module, use `ctest -L "^IMP\.atom\-test\-.*EXPENSIVE"`.

Benchmarks are simply tests labeled as `benchmark`; examples are tests labeled as `example`.

Note that some test failures are to be expected; compare the failures with
those at our own [nightly builds page](http://integrativemodeling.org/nightly/results/)
if you are concerned.

## Installation {#installation_install}

Once everything is compiled (and optionally tested) you can install %IMP
by simply running `make install`. If you opted to install in a non-standard
location, it is up to you to set up your environment variables so that %IMP
can be found (you may need to set `PATH`, `PYTHONPATH`, and `LD_LIBRARY_PATH`).
 
Alternatively, you can run %IMP directly from the build directory by using
the `setup_environment.sh` script. This sets the necessary environment
variables and then runs the rest of the command line with this modified
environment. For example, to run the `ligand_score` command line tool you
can either run

    ./setup_environment.sh ligand_score <arguments>

or create a new shell with

    ./setup_environment.sh $SHELL

and then run

    ligand_score <arguments>

in that shell.
