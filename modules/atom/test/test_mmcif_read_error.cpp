/**
 *  \file test_mmcif_read_error.cpp
 *  \brief Test that errors reading from a stream are reported
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/atom/mmcif.h>
#include <IMP/exception.h>
#include <IMP/Model.h>
#include <IMP/Pointer.h>
#include <IMP/flags.h>
#include <iostream>
#include <istream>
#include <stdexcept>
#include <streambuf>

namespace {

// A stream buffer that fails on any read. Unlike (for example) reading from
// a directory, which sets different stream flags on different platforms,
// istream::read() must set badbit if the stream buffer throws an exception.
class FailingStreambuf : public std::streambuf {
 protected:
  int_type underflow() override { throw std::runtime_error("read failed"); }
};

}  // namespace

int main(int argc, char *argv[]) {
  IMP::setup_from_argv(argc, argv, "Test mmCIF stream read errors");
  IMP_NEW(IMP::Model, m, ());
  FailingStreambuf buf;
  std::istream in(&buf);
  try {
    IMP::atom::read_mmcif(in, m);
  } catch (const IMP::IOException &) {
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Wrong exception type: " << e.what() << std::endl;
    return 1;
  }
  std::cerr << "No exception raised" << std::endl;
  return 1;
}
