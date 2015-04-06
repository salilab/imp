Cross platform compatibility {#cross_platform}
============================

%IMP is designed to run on a wide variety of platforms. To detect problems on
other platforms we provide [nightly test runs](http://integrativemodeling.org/nightly/results/)
on the supported platforms for code that is part of the %IMP repository.

In order to make it more likely that your code works on all the supported platforms:
- avoid the use of `and` and `or` in C++ code; use `&&` and `||` instead.
- avoid `friend` declarations involving templates; use the preprocessor,
  conditionally on the symbols `SWIG` and `IMP_DOXYGEN` to hide code as
  needed instead.
- don't use Python modules that aren't available in Python 2.6 (the oldest
  version of Python supported by IMP). For example, avoid `argparse`
  (use `optparse` instead).
- try to write Python code that also works in Python 3 (IMP supports both
  Python 2 and Python 3 in one codebase). Most
  obviously, in Python 3, `print` is a function, not a statement. So write
  `print("foo")` rather than `print "foo"` - the former works in Python 2 too.
  To catch a lot of Python 3-incompatible code quickly, add
  `from __future__ import print_function, division, absolute_import` at the
  very top of your Python submodules (top-level modules have this already).
- if you must use an external C++ library, it needs to have a very permissive
  open source license (such as BSD or LGPL - not GPL) and note that this
  will reduce the number of potential users of your code (since it's another
  dependency to find).
- try to avoid Linux- or Mac-centric coding that won't work on Windows.
  For example, use `os.path.join` to join paths in Python, rather than
  adding the '/' character. Write utility scripts in Python (which is
  available on Windows, since the rest of IMP requires it) not as shell
  scripts, Perl scripts, etc.
