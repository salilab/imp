# Developer Tools

This project contains tools that are used by a variety of development projects. In particular it provides tools to deal with
- source code standards: `check_standards.py` checks various coding standards and `cleanup_code.py` autoformats code files (using `autopep8` and `clang-format` when available).
- generating file lists for `cmake`: `setup_cmake.py` generates `Files.cmake` lists of `.cpp` and `.py` files that `cmake` scripts can read. `make_all_header.py` generates a header that includes a list of others, eg `IMP.h`. These are useful to simplify SWIG files.
- replacing strings in source files: `replace.py` does find and replace, only modifying files that are changed (sed touches all passed files)
- git support: see the git subdir
