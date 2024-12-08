Cross platform compatibility {#cross_platform}
============================

%IMP is designed to run on a wide variety of platforms. To detect problems on
other platforms we provide [nightly test runs](https://integrativemodeling.org/nightly/results/)
on the supported platforms for code that is part of the %IMP repository.

In order to make it more likely that your code works on all the supported platforms:
- avoid the use of `and` and `or` in C++ code; use `&&` and `||` instead.
- avoid `friend` declarations involving templates; use the preprocessor,
  conditionally on the symbols `SWIG` and `IMP_DOXYGEN` to hide code as
  needed instead.
- don't use Python modules or functions that aren't available in Python 3.6
  (the oldest version of Python supported by %IMP).
- if you must use an external C++ library, it needs to have a very permissive
  open source license (such as BSD or LGPL - not GPL) and note that this
  will reduce the number of potential users of your code (since it's another
  dependency to find).
- try to avoid Linux- or Mac-centric coding that won't work on Windows.
  For example, use `pathlib.Path` or `os.path.join` to join paths in Python,
  rather than adding the '/' character. Write utility scripts in Python
  (which is available on Windows, since the rest of %IMP requires it) not
  as shell scripts, Perl scripts, etc.
