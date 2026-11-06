/**
 *  \file test_charmm_read_error.cpp
 *  \brief Test that errors reading CHARMM files are reported
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/atom/CHARMMParameters.h>
#include <IMP/exception.h>
#include <IMP/Pointer.h>
#include <IMP/file.h>
#include <IMP/flags.h>
#include <iostream>
#include <istream>
#include <sstream>
#include <stdexcept>
#include <streambuf>

namespace {

// A stream buffer that fails on any read. Unlike (for example) reading from
// a directory, which sets different stream flags on different platforms,
// std::getline() must set badbit if the stream buffer throws an exception.
class FailingStreambuf : public std::streambuf {
 protected:
  int_type underflow() override { throw std::runtime_error("read failed"); }
};

bool check_read_error(IMP::TextInput top, IMP::TextInput par) {
  try {
    IMP_NEW(IMP::atom::CHARMMParameters, ff, (top, par));
  } catch (const IMP::IOException &) {
    return true;
  } catch (const std::exception &e) {
    std::cerr << "Wrong exception type: " << e.what() << std::endl;
    return false;
  }
  std::cerr << "No exception raised" << std::endl;
  return false;
}

}  // namespace

int main(int argc, char *argv[]) {
  IMP::setup_from_argv(argc, argv, "Test CHARMM file read errors");
  // Skip test on Windows; we usually run under WINE and their stream
  // implementation doesn't catch exceptions like it is supposed to
#if !defined(_WIN32) && !defined(_WIN64)
  FailingStreambuf buf;
  std::istream bad_top(&buf), bad_par(&buf);
  std::istringstream good_top(""), good_par("");
  if (!check_read_error(IMP::TextInput(bad_top), IMP::TextInput(good_par))) {
    return 1;
  }
  if (!check_read_error(IMP::TextInput(good_top), IMP::TextInput(bad_par))) {
    return 1;
  }
#endif
  return 0;
}
