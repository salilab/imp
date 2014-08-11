Installation {#installation}
============

# Building and installing basics #

[TOC]

IMP is available in a variety of different ways. If you are just planning on using
existing IMP code and run on a standard platform, you may be able to install
a pre-built binary. See the [download](http://integrativemodeling.org/download.html) page.

If you are planning on contributing to IMP, you should download and build the source.
See the next section for more information.



Building IMP from source is straightforward if the [prerequisites](#prereqs)
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


# Prerequisites # {#installation_prereqs}
In order to obtain and compile IMP, you will need:

- cmake (2.8 or later)
- Boost (1.40 or later)
- HDF5 (1.8 or later)
- Developers will also need a git client to access the repository

If you wish to build the Python interfaces, you will also need:

- Python (2.4 or later)
- SWIG (1.3.40 or later)

## Getting prerequisites on a Mac ## {#installation_prereqs_mac}

Mac users must first install Xcode (previously known as Developer Tools)
which is not installed by default with OS X, but is available from the App store
(or from the Mac OS install DVD for old versions of Mac OS). They will also
need the Xcode command line tools (install by going to Xcode Preferences, then
Downloads, then Components, and select "Command Line Tools").

Then Mac users should use one of the available collections of Unix tools,
either
- [Homebrew](https://github.com/mxcl/homebrew) (_recommended_) Once you installed `homebrew`
  do

    `brew tap homebrew/science`

    `brew install boost gmp google-perftools cgal graphviz gsl cmake doxygen hdf5 swig eigen fftw mpfr`

  to install everything IMP finds useful (or that you will want for installing various useful python libs that IMP finds useful). On older Macs, you may also need to `brew install git` if you want to use git (newer Macs include git).
- [Macports](http://www.macports.org/) If you use MacPorts, you must add `/opt/local/bin` to your path (either by modifying your shell's
  config file or by making an `environment.plist` file) and then do

    `sudo port install boost cmake swig-python`

  to install the needed libraries and tools. When installing HDF5 with MacPorts, be sure to install `hdf5-18`
  (version 1.8), rather than the older `hdf5` (version 1.6.9).
- or [Fink](http://www.finkproject.org/)

### Mac OS X 10.5 and 10.6
These versions of mac os include a 'swig' binary, but it is too old to use
with IMP. You need to make sure that the newer version of `swig` is found first
in your `PATH`.


## Getting prerequisites on Windows ## {#installation_prereqs_windows}

We recommend Linux or Mac for developing with IMP, as obtaining the
prerequisites on Windows is much more involved. However, we do test IMP on
Windows, built with the Microsoft Visual Studio compilers (we use Visual Studio
Express 2010 SP1 for 32-bit Windows, and VS Express 2012 for 64-bit).
One complication is that different packages are compiled
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
  - Get and install the
    [zlib package](http://gnuwin32.sourceforge.net/packages/zlib.htm)
    (both the "complete package, except sources" and the "sources" installers).
     - The package without sources can be installed anywhere; we chose the
       default location of `C:\Program Files\GnuWin32`. The sources, however,
       must be installed in a path that doesn't contain spaces (otherwise the
       Boost build will fail). We chose `C:\zlib`.
     - We found that the zconf.h header included with zlib erroneously includes
       unistd.h, which doesn't exist on Windows, so we commented out that line
       (in both packages).
  - Download the [Boost source code](http://www.boost.org)
    (we extracted it into `C:\Program Files\boost_1_53_0`), then
     - Open a Visual Studio Command Prompt, and cd into the directory where
       Boost was extracted
     - Run bootstrap.bat
     - You may need to help the compiler find the zlib header file with
       `set INCLUDE=C:\Program Files\GnuWin32\include`
     - Run `bjam link=shared runtime-link=shared -sNO_ZLIB=0 -sZLIB_SOURCE=C:\zlib\1.2.3\zlib-1.2.3`
  - Get and install [SWIG for Windows](http://www.swig.org)
  - Get the [HDF5 source code](http://www.hdfgroup.org)
     - Make a 'build' subdirectory, then run from a command prompt in
       that subdirectory something similar to
       `cmake.exe -G "Visual Studio 10" -DHDF5_ENABLE_SZIP_SUPPORT:BOOL=OFF -DHDF5_ENABLE_Z_LIB_SUPPORT:BOOL=ON -DHDF5_BUILD_HL_LIB:BOOL=ON -DZLIB_INCLUDE_DIR="C:\Program Files\GnuWin32\include" -DZLIB_LIBRARY="C:\Program Files\GnuWin32\lib\zlib.lib" -DBUILD_SHARED_LIBS:BOOL=ON ..`
     - Open the resulting HDF5 solution file in Visual Studio, change to
       Release configuration, then build the hdf5 project.
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
    [libTAU](http://integrativemodeling.org/libTAU.html)
     - Copy `libTAU.lib` to `TAU.lib` to help cmake find it.
  - (Optional) Get the [OpenCV source code](http://opencv.org/)
    and build it by [following these instructions](http://docs.opencv.org/doc/tutorials/introduction/windows_install/windows_install.html)
     - Copy each `opencv_*.lib` to a similar file without the version extension
       (e.g. copy `opencv_ml244.lib` to `opencv_ml.lib`) to help cmake find it
  - Set PATH, INCLUDE, and/or LIB environment variables so that the compiler
    can find all of the dependencies. (We wrote a little batch file.)
  - Set up IMP by running something similar to

     `cmake <imp_source_directory> -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="/DBOOST_ALL_DYN_LINK /EHsc /D_HDF5USEDLL_ /DWIN32 /DGSL_DLL" -G "NMake Makefiles"`

  - Note: if building for 64-bit Windows, you may need to add `/bigobj` to `CMAKE_CXX_CFLAGS`.
  - Then use simply 'nmake' (instead of 'make', as on Linux or Mac) to
    build IMP. (cmake can also generate Visual Studio project files, but
    we recommend nmake.)
  - To use IMP or run tests, first run the setup_environment.bat file to set
    up the environment so all the programs and Python modules can be found.
    (This batch file needs to be run only once, not for each test.)

## Getting prerequisites on Linux
All of the prerequisites should be available as pre-built packages for
your Linux distribution of choice.

# Optional prerequisites # {#installation_preqs_optional}

IMP can make use of a variety of external tools to provide more or
better functionality.

- [Doxygen](http://www.doxygen.org/) and [Graphviz](http://www.graphviz.org/): Required for building documentation. They are available as
part of most Unix tool sets (HomeBrew, all Linux distributions etc.).
- [Modeller](\ref modeller)
- [CGAL](\ref CGAL)
- [Google perf tools](\ref perf)
- [ANN](\ref ANN)
- [GSL](\ref GSL)
- [OpenCV](\ref OpenCV) is needed for IMP.em2d
- [libTAU](http://integrativemodeling.org/libTAU.html) is needed for IMP.cnmultifit
- [MPI](\ref impmpi)

# Where to go next # {#installation_next}

You are now ready to use IMP within Python and C++.

Everyone should read the [introduction](\ref introduction) and developers should
then move on to the [Developer Guide](\ref devguide).
