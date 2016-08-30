[![Build Status](https://travis-ci.org/salilab/developer_tools.svg?branch=master)](https://travis-ci.org/salilab/developer_tools)
[![Coverage Status](https://coveralls.io/repos/github/salilab/developer_tools/badge.svg?branch=master)](https://coveralls.io/github/salilab/developer_tools?branch=master)
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

Most of the scripts are Python scripts, and should work with Python 2.6 or
later (including Python 3).
