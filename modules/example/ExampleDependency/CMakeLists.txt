cmake_minimum_required(VERSION 2.8)
project(example_dependency)
add_library(example_dependency SHARED example_dependency_lib.cc example_dependency_header.hh)
