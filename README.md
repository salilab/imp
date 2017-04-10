[![nightly build](https://integrativemodeling.org/nightly/results/?p=stat)](https://integrativemodeling.org/nightly/results/)
[![coverity scan](https://img.shields.io/coverity/scan/8505.svg)](https://scan.coverity.com/projects/salilab-imp)

For full installation and usage instructions, see the
[documentation](https://integrativemodeling.org/nightly/doc/manual/).

Building IMP from source code
=============================

To build IMP source found in `path/to/imp-source` and install it in
`path_to_install` do:

1. `mkdir build && cd build`
2. `cmake path/to/imp-source -DCMAKE_INSTALL_PREFIX=path_to_install`
3. `make -j4`
4. `make install`

See the [installation instructions in the manual](https://integrativemodeling.org/nightly/doc/manual/installation.html)
for more details (in particular this covers the necessary prerequisites you'll
need before running cmake).

Run `ccmake` to see more variables that can be used to customize your build and
install.

To run IMP without installing, build IMP and then use `setup_environment.sh` to
set the required environment variables. For example, to run `ligand_score`, do
`./setup_environment.sh ligand_score arguments...`, or
`./setup_environment.sh $SHELL` and then `ligand_score arguments...` in the
new shell.

Copyright and License information
=================================

IMP is Copyright 2007-2017 IMP Inventors. The IMP Inventors are
Andrej Sali, Ben Webb, Daniel Russel, Keren Lasker, Dina Schneidman,
Javier Velázquez-Muriel, Friedrich Förster, Elina Tjioe, Hao Fan,
Seung Joong Kim, Yannick Spill, Riccardo Pellarin.

IMP is largely available under the GNU Lesser GPL; see the file COPYING.LGPL
for the full text of this license. Some IMP modules are available under the
GNU GPL (see the file COPYING.GPL). Please refer to the
[documentation](https://integrativemodeling.org/nightly/doc/manual/licenses.html)
for more detail.

Files and folders structure
===========================

When you build IMP, the `lib` directory contains Python modules (both pure
Python, in the `IMP` subdirectory, and C++ extensions, as `_IMP_foo.so`
in the top-level directory) and the IMP C++ libraries (as `libimp_foo.so`
in the top-level directory). Rationale: Windows searches for C++ dynamic
libraries in the same directory as C++ Python extensions.

The `include` directory contains all public header files under the `IMP`
subdirectory. Modules are in named subdirectories under that.

The `swig` directory contains all SWIG interface (`.i`) files used to build
the Python interface.
