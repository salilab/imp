Tools for developers {#dev_tools}
====================

%IMP provides a variety of scripts to aid the lives of developers.

# Making a module # {#dev_tools_make_module}

Creating a module is the easiest way to get started developing
code for %IMP. First, choose a name for the module.  The name should
only contain letters, numbers and underscores as it needs to be a
valid file name as well as an identifier in Python and C++.

To create the module run `tools/make-module.py my_module` from the top-level
%IMP directory. The new module includes a number of examples and comments
to help you add code to the module. The directory structure of the new module
is identical to [that of existing modules](@ref directories).

If you feel your module is of interest to other %IMP users and
developers, see the [contributing code to IMP](#devguide_contributing) section.

# Formatting your code # {#dev_tools_cleanup_code}

The `tools/dev_tools/cleanup_code.py` script uses the external
`clang-format` and `autopep8` programs to reformat C++ and Python code
respectively to conform to our [preferred indentation](@ref codeconv_indentation), working around some eccentricities of %IMP code. `clang-format`
is part of [llvm](http://llvm.org) >= 3.4. You should always inspect the
changes made by `clang-format` before submitting.

# Checking standards # {#dev_tools_check_standards}

The `tools/dev_tools/check_standards.py` runs a number of
%IMP-specific standards checks on C++ and Python files.
It is also run as part of `git` commits. This can help you conform to %IMP's
[code conventions](@ref code_conventions).
