Python-only modules {#pyonlymod}
===================

Most %IMP modules are mostly written in C++ and provide a Python wrapper.
However, it is also possible to write modules entirely in Python. To do this,

- add `python_only=True` to the module's `dependencies.py` file.
- don't provide a SWIG input file (`swig.i-in`) in the module's `pyext`
  directory (it will be ignored).
- provide an `__init__.py` in the module's `pyext/src` directory if needed.
- any programs in the module's `bin` subdirectory should be Python scripts.
- utility C++ headers can still be provided in the module's `include` directory
  but no `.cpp` files can be included in the `src` directory.

The advantage of making a Python-only module is that it is much faster to
build (since it does not need to be compiled).

For good examples, see the `test`, `sampcon`, and `saxs_merge` modules.
