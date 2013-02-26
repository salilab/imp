/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */
#include <RMF/FileHandle.h>
#include <RMF/NodeConstHandle.h>
#include <boost/algorithm/string/predicate.hpp>
#include <cstdio>
#include <boost/filesystem.hpp>
#include "common.h"
namespace {
std::string description("Try to upgrade an out of date RMF file.");
int frame = 0;
}
int main(int argc, char **argv) {
  try {
    RMF_ADD_INPUT_FILE("rmf");
    process_options(argc, argv);
    if (boost::algorithm::ends_with(input, ".rmf2")
        && boost::filesystem::exists(boost::filesystem::path(input)/"file")) {
      boost::filesystem::rename(boost::filesystem::path(input)/"file",
                                boost::filesystem::path(input)/"file.rmf2info");
      std::cout << "Updated" << std::endl;
    } else {
      std::cout << "Nothing to do" << std::endl;
    }
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
  return 0;
}
