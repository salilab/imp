Directory structure {#directories}
===================

The input files in the %IMP repository are structured as follows:
- `tools` contains various command line utilities for use by developers. They
  are [documented below](#devguide_scripts).
- `doc` contains inputs for general %IMP overview documentation (such as this
  page), as well as configuration scripts for `doxygen`.
- each subdirectory of `modules/` defines a module; they all have the same
  structure. The directory for module `name` has the following structure:
   - `%README.md` contains a module overview
   - `include` contains the C++ header files
   - `src` contains the C++ source files
   - `bin` contains C++ source files each of which is built into an executable,
     and/or Python scripts. These programs will be installed for %IMP users
     to use
   - `utility` is like `bin` but the programs are not installed - they are only
     for use during the build procedure
   - `pyext` contains files defining the Python interface to the module as well
      as Python source files (in `pyext/src`)
   - `test` contains test files that can be run with `ctest`
   - `doc` contains additional documentation that is provided via `.dox`
     or `.md` files
   - `examples` contains examples in Python and C++, as well as any data needed
      for examples
   - `data` contains any data files needed by the module

When %IMP is built, a number of directories are created in the build directory.
They are
 - `include` which includes all the headers. The headers for module `name` are
    placed in `include/IMP/name`
 - `lib` where the C++ and Python libraries are placed. Module `name` is built
    into a C++ library `lib/libimp_name.so` (or `.dylib` on a Mac) and a Python
    library with Python files located in `lib/IMP/name` and the binary part in
    `lib/_IMP_name.so`
 - `doc` containing the manual in `doc/html` and the examples
    in `doc/examples` with a subdirectory for each module
 - `data` where each module gets a subdirectory for its data.

When %IMP is installed, the structure from the build directory is
moved over more or less intact except that the C++ and Python
libraries are put in the (different) appropriate locations.
