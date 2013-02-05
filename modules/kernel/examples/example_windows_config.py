## \example example_windows_config.py
## A config for windows. It probably doesn't work.

cxxflags="/DWIN32 /DGSL_DLL /D_HDF5USEDLL_"  # Needed to link with
                                             # GSL and HDF5 DLLs
boost_autolink="dynamic"        # Needed to link with Boost DLLs
fftw3="yes"
fftw3libs="libfftw3-3.lib"      # Needed to link with FFTW3 DLL
