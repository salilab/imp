Building modules out of tree {#outoftree}
============================

Normally to build an %IMP module it is placed in the `modules` subdirectory
together with the rest of the %IMP source code and then all of %IMP is
built, as per the [installation instructions](@ref installation_source).
However, this makes building a module an expensive process (as the build
system needs to check whether any part of %IMP has changed, not just the
module itself) and it is awkward to build a module against multiple
versions of %IMP, for example against both the latest stable release and
a more recent nightly build.

An alternative is to build one or more %IMP modules 'out of tree', treating
them as their own [CMake](https://cmake.org/) projects in their own
directories, and pointing them to existing %IMP installation(s) using
%IMP's [existing CMake configuration](@ref uselibrary).

## Module layout

Out of tree modules can be maintained in one of two ways:

  1. A top-level `modules` directory containing a number of subdirectories,
     one per module; the name of each subdirectory is the name of the module.
     This mimics the way %IMP itself maintains multiple modules.
  2. All of a single module's files can be placed in the top-level directory,
     as is done for the [PMI](https://github.com/salilab/pmi/) module. This
     has the advantage that the module can also be cloned directly into
     %IMP's `modules` directory and built 'in tree', as well.

## CMake configuration

Place the [FindIMP.cmake](https://github.com/salilab/pmi/blob/develop/tools/FindIMP.cmake)
file into a suitable directory, say a `tools` subdirectory. This file will help
CMake to find the %IMP installation, as [described earlier](@ref uselibrary).

Make a top-level `CMakeLists.txt` file. This file
 - Adds the `tools` directory to the CMake search path, so that CMake can
   find %IMP;
 - Finds the %IMP package;
 - Uses the `imp_build_module` CMake macro, provided in %IMP's CMake
   configuration, to build the module;
 - For the single module case, detect an 'in tree' build and fall back to
   using %IMP's own CMake scripts.

A suitable `CMakeLists.txt` can be found in the
[PMI repository](https://github.com/salilab/pmi/blob/develop/CMakeLists.txt) -
all that needs to be changed is `CMAKE_MODULE_PATH` (if `FindIMP.cmake` was
not put in the `tools` directory) and the second argument to
`imp_build_module`, which is the module name. (If multiple modules are
being built under the `modules` directory, this second argument should
be omitted - the module names will match those of the subdirectories.)

If you also need to use [RMF](https://integrativemodeling.org/rmf/)
this should be searched for separately; add the
[FindRMF.cmake](https://github.com/salilab/npctransport/blob/develop/tools/FindRMF.cmake)
file and add a suitable `find_package` call to `CMakeLists.txt` after the
search for %IMP. A suitable `CMakeLists.txt` can be found in the
[npctransport repository](https://github.com/salilab/npctransport/blob/develop/CMakeLists.txt).
This is needed because while RMF is often bundled with %IMP, it can also be
installed separately (this is the case, for example, with some of the
Anaconda packages).

## Building

The module can now be built like any other CMake project - i.e. make a
build directory, and run CMake in that build directory, giving it the
path to the module source code. (You may need to set the `IMP_DIR` variable
to help CMake find %IMP.)

## Testing and running

After a successful build, the build directory will contain a
`setup_environment.sh` script. This can be used just like that in %IMP
itself. It will add both the out of tree module(s) and %IMP to the search
path so that the module can be tested or used.
