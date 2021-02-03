[![Build Status](https://github.com/salilab/developer_tools/workflows/build/badge.svg?branch=main)](https://github.com/salilab/developer_tools/actions?query=workflow%3Abuild)
[![codecov](https://codecov.io/gh/salilab/developer_tools/branch/main/graph/badge.svg)](https://codecov.io/gh/salilab/developer_tools)
[![Code Climate](https://codeclimate.com/github/salilab/developer_tools/badges/gpa.svg)](https://codeclimate.com/github/salilab/developer_tools)

# Developer Tools

This project contains tools that are used by a variety of development projects.
In particular it provides tools to deal with

- source code standards: `check_standards.py` checks various coding standards
  and `cleanup_code.py` autoformats code files (using `autopep8` and
  `clang-format` when available).
- generating file lists for `cmake`: `setup_cmake.py` generates `Files.cmake`
  lists of `.cpp` and `.py` files that `cmake` scripts can read.
  `make_all_header.py` generates a header that includes a list of others,
  eg `IMP.h`. These are useful to simplify SWIG files.
- replacing strings in source files: `replace.py` does find and replace,
  only modifying files that are changed (sed touches all passed files).
- git support: see the `git` subdir.

Most of the scripts are Python scripts, and should work with Python 2.7 or
later (including Python 3).
