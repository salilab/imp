The installer will install IMP under a directory of your choosing.
For example, the FoXS fitting program is installed in the 'bin' subdirectory,
so can be run simply by opening a Command Prompt, changing to this bin
directory (or adding it to your PATH), and typing 'foxs'.

The IMP Python libraries are automatically set up to work with Python 2.7,
3.4, 3.5, or 3.6. (You need to download Python separately from www.python.org
and install it; be sure to get the 64-bit version of Python if you installed
the 64-bit version of IMP, and likewise for the 32-bit version - you can't
mix and match.)

One easy way to run IMP Python scripts is to open them in
Python's IDLE program and run them by pressing F5.

IMP is built with MPI support using Microsoft MPI. To use MPI-enabled IMP
components (such as the IMP.mpi and IMP.spb modules, and the spb* programs)
install MS MPI from https://msdn.microsoft.com/en-us/library/bb524831.aspx
and use its mpiexec utility to run IMP. If you do *not* want to use MPI, the
rest of IMP should work fine without needing MS MPI installed.
