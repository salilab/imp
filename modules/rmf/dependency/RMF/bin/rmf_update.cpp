/**
 * Copyright 2007-2021 IMP Inventors. All rights reserved.
 */
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <exception>
#include <iostream>
#include <string>

#include "common.h"

namespace {
std::string description("Try to upgrade an out of date RMF file.");
}
int main(int argc, char** argv) {
  try {
    RMF_ADD_INPUT_FILE("rmf");
    process_options(argc, argv);
    if (boost::algorithm::ends_with(input, ".rmf2") &&
        boost::filesystem::exists(boost::filesystem::path(input) / "file")) {
      boost::filesystem::rename(
          boost::filesystem::path(input) / "file",
          boost::filesystem::path(input) / "file.rmf2info");
      std::cout << "Updated" << std::endl;
    } else {
      std::cout << "Nothing to do" << std::endl;
    }
  }
  catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
  return 0;
}
