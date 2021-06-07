Adding external dependencies {#extdepends}
============================

[TOC]

# Introduction # {#extdep_intro}

You can have an %IMP module depend on some external library, for example
to take advantage of a method implemented in that library, or to link
%IMP with another piece of software at runtime.

# Considerations # {#extdep_consider}

Think very carefully before introducing a new external dependency. Each
additional dependency makes it harder for people to use your code.
If you do need to add a dependency, it needs to be open source and available
under a suitably permissive license (for %example if it is available under
the GPL then you cannot license your module as LGPL, but will need to also
make it GPL).

Generally if you need to add a new dependency you should probably also put
your code in a new module, rather than adding it to an existing module. That
way, people that elect not to install that dependency will only be deprived
of your code, not of the existing module.

# Simple dependencies # {#extdep_simple}

The simplest way to add a C/C++ dependency `foo` is to create a file
`foo.description` in the `dependency` subdirectory of your module. This is a
simple text file containing several variables:

- `headers`: a colon-separated list of any C/C++ headers that need to be
  included to use the dependency.
- `libraries`: a colon-separated list of any libraries that need to be
  linked against to use the dependency. (There is a similar `extra_libraries`
  variable, if needed, for libraries that aren't part of the dependency but
  that need to also be linked in order for it to work.)
- `body`: a fragment of C++ code that uses the dependency.

When CMake is run, it will use the variables in `foo.description` to build
a small test program in order to make sure the dependency is available and
that it works. See `modules/kernel/dependency/GPerfTools.description` for
an example.

# More complex dependencies # {#extdep_complex}

For more complex dependencies, you can also create a `foo.cmake` file
containing arbitrary CMake instructions to configure the dependency. See for
example `modules/rmf/dependency/RMF.cmake`.

# Using the dependency # {#extdep_using}

The next step in adding a `foo` dependency is to list `foo` in the module's
`dependencies.py` file. `foo` can be listed either in that file's
`required_dependencies` variable or in `optional_dependencies`, both of
which are colon-separated lists of external dependencies. See for example
`modules/kernel/dependencies.py`.

If placed in `required_dependencies`, the module cannot be built unless your
dependency is found. The module will be automatically linked against the
dependency.

If placed in `optional_dependencies`, the module can be built either with
or without the dependency. If the dependency is available, a preprocessor
macro will be set when the module is built; protect your code with that
macro. For example, IMP::score_functor can be built with or without the
HDF5 library. Any code that requires HDF5 is conditional on the
`IMP_SCORE_FUNCTOR_USE_HDF5` preprocessor macro.
