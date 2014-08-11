Tools {#dev_tools}
=====

# Tools #

IMP provides a variety of scripts to aid the lives of developers.

# Making a module # {#dev_tools_make_module}

Creating such a module is the easiest way to get started developing
code for IMP. First, choose a name for the module.  The name should
only contain letters, numbers and underscores as it needs to be a
valid file name as well as an identifier in Python and C++.

To create the module do `./tools/make-module.py my_module`. The new
module includes a number of examples and comments to help you add code
to the module.

You can use your new module in a variety of ways:
- add C++ code to your module by putting `.h` files in
  `modules/my_module/include` and `.cpp` files in
  `modules/my_module/src`. In order to use use your new
  functions and classes in Python, you must add a line
  `%include "IMP/my_module/myheader.h"` near the end of the
  file `modules/my_module/pyext/my_module.i`.
- write C++ programs using IMP by creating `.cpp` files in
      `modules/my_module/bin`. Each `.cpp` file placed there
      is built into a separate executable.
- add Python code to your library by putting a `.py` file in
      `modules/my_module/pyext/my_module/`
- add Python code to your library by by adding
      `%pythoncode` blocks to `modules/my_module/pyext/my_module.i`.
- add test code to your library by putting `.py` files in
      `modules/my_module/test` or a subdirectory.

If you feel your module is of interest to other IMP users and
developers, see the [contributing code to IMP](#devguide_contributing) section.

If you document your code, building the target `IMP-doc` will build
documentation of all of the modules including yours and
`IMP.mymodule-doc` will build the doc for just yours. To access the
documentation for all of IMP, open `doc/html/index.html` and for just
your module, open `doc/html/mymodule/index.html`

# Formatting your code # {#dev_tools_clang_format}

The command `./tools/clang-format.py` uses the program `clang-format` to
reformat C++ code, working around some eccentricies of IMP code. `clang-format`
is part of [llvm](http://llvm.org) >= 3.3. You should always inspect the
changes made by `clang-format` before submitting.

# Checking standards # {#dev_tools_check_standards}

The command `./tools/check-standards.py` runs a number of IMP-specific standards
checks on C++ and Python files. It is also run as part of `git` commits.
