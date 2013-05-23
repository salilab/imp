# Installation
[TOC]

IMP is available in a variety of different ways. If you are just planning on using
existing IMP code and run on a standard platform, you may be able to install
a pre-built binary. See the \salilab{imp/download.html,download page}.

If you are planning on contributing to IMP, you should download and build the source.
See the next section for more information.


## Building and installing basics

Building IMP from source is straightforward if the \ref prereq "prerequisites"
are already installed.

    git clone git://github.com/salilab/imp.git
    cd imp
    ./setup_git.py
    mkdir ../imp_release
    cd ../imp_release
    cmake ../imp -DCMAKE_BUILD_TYPE=Release
    make -j 8

See [Building IMP with CMake](https://github.com/salilab/imp/wiki/Cmake)
for more information.


## Prerequisites
In order to obtain and compile IMP, you will need:

- cmake (2.8 or later)
- Boost (1.40 or later)
- HDF5 (1.8 or later)
- Developers will also need a git client to access the repository

If you wish to build the Python interfaces, you will also need:

- Python (2.4 or later)
- SWIG (1.3.40 or later)

## Getting prerequisites on a Mac

Mac users must first install Xcode (previously known as Developer Tools)
which is not installed by default with OS X, but is available from the App store
(or from the Mac OS install DVD for old versions of Mac OS).

Then Mac users should use one of the available collections of Unix tools,
either
- [Homebrew](http://mxcl.github.com/homebrew/) (_recommended_) Once you installed `homebrew`
  do

    brew install boost gmp google-perftools cgal graphviz gsl cmake doxygen hdf5 swig eigen fftw mpfr

  to install everything IMP finds useful (or that you will want for installing various useful python libs that IMP finds useful).
- [Macports](http://www.macports.org/) If you use MacPorts, you must add `/opt/local/bin` to your path (either by modifying your shell's
  config file or by making an `environment.plist` file) and then do

    sudo port install boost cmake swig-python

  to install the needed libraries and tools. When installing HDF5 with MacPorts, be sure to install `hdf5-18`
  (version 1.8), rather than the older `hdf5` (version 1.6.9).
- or [Fink](http://www.finkproject.org/)

### Mac OS X 10.5 and 10.6
These versions of mac os include a 'swig' binary, but it is too old to use
with IMP. You need to make sure that the newer version of `swig` is found first
in your `PATH`.


## Getting prerequisites on Windows

We recommend Linux or Mac for developing with IMP, as obtaining the
prerequisites on Windows is much more involved. However, we do test IMP on
Windows, built with the Microsoft Visual Studio compilers (we use Visual Studio
Express 2010 SP1). One complication is that different packages are compiled
with different versions of Visual Studio, and mixing the different runtimes
(msvc*.dll) can cause odd behavior; therefore, we recommend building most
of the dependencies from source code using the same version of Visual Studio
that you're going to use to build IMP. The basic procedure is as follows:

  - Install Microsoft Visual Studio Express (it is free, but registration with
    Microsoft is required).
  - Get and install [cmake](http://www.cmake.org).
  - Get [Python 2](http://www.python.org) (not Python 3)
    (make sure you get the
    32-bit version if you're going to build IMP for 32-bit Windows).
  - Download the [Boost source code](http://www.boost.org)
    (we extracted it into `C:\Program Files\boost_1_53_0`), then
     - Open a Visual Studio Command Prompt, and cd into the directory where
       Boost was extracted
     - Run bootstrap.bat
     - Run `bjam link=shared runtime-link=shared`
  - Get and install [SWIG for Windows](http://www.swig.org)
  - Get and install the
    [zlib package](http://gnuwin32.sourceforge.net/packages/zlib.htm)
    (complete package without sources).
     - We found that the zconf.h header included with zlib erroneously includes
       unistd.h, which doesn't exist on Windows, so we commented out that line.
  - Get the [HDF5 source code](http://www.hdfgroup.org)
     - Edit the H5pubconf.h file in the windows\src subdirectory to
       disable szip (or first install szip if you want to include szip support).
       Copy this file into the top-level src directory.
     - Open the h5libsettings project (in `windows\misc\typegen\h5libsettings`)
       in Visual Studio, and build it in Release configuration. (Note that if
       you don't have the x64 SDK installed, you'll first need to edit the
       project file in a text editor and remove any mention of the x64 platform,
       since otherwise the upgrade of this solution to 2010 format will fail.)
     - Build the 'h5tinit' project (in `windows\misc\typegen\h5tinit`) in
       Release configuration.
     - Build the 'hdf5dll' project (in `windows\proj\hdf5dll`) in
       Release configuration.
        - In order for Visual Studio to find zlib, we first opened the project
          settings, and under C/C++, Additional Include Directories, added
          `C:\Program Files\GnuWin32\include`, and under Linker, Input,
          Additional Dependencies, added
          `C:\Program Files\GnuWin32\lib\zlib.lib`.
     - Copy proj\hdf5dll\Release\hdf5dll.lib to hdf5.lib to help cmake
       find it.
  - (Optional) [Build CGAL from source code](http://www.cgal.org/windows_installation.html).
  - (Optional) [Get and install Eigen](http://eigen.tuxfamily.org).
  - (Optional) Download the
    [FFTW DLLs](http://www.fftw.org/install/windows.html) and follow the
    instructions at that website to make .lib import libraries needed for
    Visual Studio.
     - Copy `libfftw3-3.lib` to `fftw3.lib` to help cmake find it
  - (Optional) Get the
    [GSL source code](http://gnuwin32.sourceforge.net/packages/gsl.htm)
    and build it:
     - Open the libgsl project file in the `src\gsl\1.8\gsl-1.8\VC8`
       subdirectory
     - Build in Release-DLL configuration
     - Copy the generated `libgsl.dll` and `libgslcblas.dll` to a suitable
       location (we used `C:\Program Files\gsl-1.8\lib`)
     - Copy the corresponding .lib files, libgsl_dll.lib and libgslcblas_dll.lib
       (we recommend removing the _dll suffix and the lib prefix when you do
       this so that cmake has an easier time finding them, i.e. call them
       gsl.lib and gslcblas.lib).
  - (Optional) Get [numpy and scipy](http://www.scipy.org) to match your
    Python version.
  - (Optional) Get and install
    [libTAU](http://salilab.org/imp/libTAU.html)
     - Copy `libTAU.lib` to `TAU.lib` to help cmake find it.
  - (Optional) Get the [OpenCV source code](http://opencv.willowgarage.com/wiki/InstallGuide)
    and build it by following the instructions at that website.
     - Copy each opencv_*.lib to a similar file without the version extension
       (e.g. copy opencv_ml244.lib to opencv_ml.lib) to help cmake find it
  - Set PATH, INCLUDE, and/or LIB environment variables so that the compiler
    can find all of the dependencies. (We wrote a little batch file.)
  - Set up IMP by running something similar to

     cmake <imp_source_directory> -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="/DBOOST_ALL_DYN_LINK /EHsc /D_HDF5USEDLL_ /DWIN32 /DGSL_DLL" -G "NMake Makefiles"

  - Then use simply 'nmake' (instead of 'make', as on Linux or Mac) to
    build IMP. (cmake can also generate Visual Studio project files, but
    we recommend nmake.)
  - To use IMP or run tests, first run the setup_environment.bat file to set
    up the environment so all the programs and Python modules can be found.
    (This batch file needs to be run only once, not for each test.)

### Getting prerequisites on Linux
All of the prerequisites should be available as pre-built packages for
your Linux distribution of choice.

### Optional prerequisites

IMP can make use of a variety of external tools to provide more or
better functionality.

#### Doxygen and Dot {#doxygen}
Building the documentation requires [Doxygen](http://www.doxygen.org/)
and
[graphviz](http://www.graphviz.org/). It is available as
part of most Unix tool sets (HomeBrew, all Linux distributions etc.).

#### MODELLER {#modeller}
If you want to use IMP with MODELLER, you should use version 9v7 or
later. If you installed the MODELLER Linux RPM or Mac package, it should be
detected automatically. Make sure that MODELLER is found in your `PYTHONPATH`.

#### CGAL {#CGAL}
A wide variety of geometric computations will be faster, better or more reliable
if CGAL is installed. CGAL is available as part of most
Linux distributions and can be built from source on Mac OS or Linux. It
is free for academic use, but commercial use requires a license. See IMP::cgal
for more information.

#### Google Perf Tools
The google perf tools can be used for cpu and memory profiling of IMP. They can
be controlled from the command line in many IMP executables.

#### ANN {#ANN}
[ ANN](http://www.cs.umd.edu/~mount/ANN) is a library implementing fast
nearest neighbor searches. Certain data structures will be faster if
it is installed. While compilation of the library from source is quite
straight forward, it is not avaible as a package for common platforms.
In addition, ANN must be built as a shared library rather than a static
library.

#### GSL {#GSL}
The IMP.gsl module requires [GSL](http://www.gnu.org/software/gsl/). It
is available as a prebuilt package on all supported platforms. Note that
GSL is distributed under the GPL and so cannot be used in C++ applications
with certain other dependencies such as CGAL.

## Where to go next

You are now ready to use IMP within Python and C++.

Everyone should read the [Introduction](introduction.html) and developers should
then move on to the [Developer Guide](devguide.html).
