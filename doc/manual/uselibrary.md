Using the library in other projects {#uselibrary}
===================================

The %IMP C++ library can be used in other projects. When %IMP is built or
installed, it creates a [CMake](https://cmake.org/) file called
`IMPConfig.cmake` which contains information about how %IMP was configured
and where all the parts of %IMP can be found. This can be used to easily
add %IMP as a dependency to other C++ projects using CMake.
When built from source, this
file is placed in the CMake build directory; when installed, it is placed
under the library directory, for example in `/usr/lib64/cmake/IMP/`.

The `IMPConfig.cmake` file defines a number of CMake variables:

- `IMP_USE_FILE`: the path to a CMake file which can be
  [included into other CMake scripts](https://cmake.org/cmake/help/v3.13/command/include.html)
  to make %IMP-related CMake functions available.
- `IMP_INCLUDE_DIR`: the path containing %IMP C++ headers.
- `IMP_xxx_LIBRARY`: the full path to the dynamic library for the `xxx` module
  (for example `IMP_kernel_LIBRARY` points to the IMP kernel and
  `IMP_atom_LIBRARY` to the IMP::atom library).
- `RMF_INCLUDE_PATH`: the path containing RMF C++ headers.
- `IMP_DATA_DIR`: the path containing %IMP data files.
- `IMP_SWIG_DIR`: the path containing SWIG `.i` files for all %IMP modules.

The paths to all %IMP dependent libraries and headers are also present in
this file - for example `EIGEN3_INCLUDE_DIR` and `Boost_INCLUDE_DIR`.

A [FindIMP.cmake](https://github.com/salilab/pmi/blob/develop/tools/FindIMP.cmake)
file is provided to help CMake find this %IMP configuration (using the
[find_package](https://cmake.org/cmake/help/v3.13/command/find_package.html)
command), and can be used in other CMake projects. 

See also the tutorial on
[using IMP as a C++ library](https://integrativemodeling.org/tutorials/using_cpp/)
for a worked example of using %IMP in a CMake project.

This functionality can also be used to build an %IMP module "out of tree",
that is to build the module by itself and link to an existing pre-built
%IMP installation. See the [out of tree](@ref outoftree) page for more
information.
