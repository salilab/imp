/**
 * Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#include <exception>
#include <iostream>
#include <string>

#include "RMF/FileConstHandle.h"
#include "RMF/show_hierarchy.h"
#include "RMF/ID.h"
#include "common.h"

namespace {
std::string description("Print out information about frames");
}

int main(int argc, char** argv) {
  try {
    RMF_ADD_INPUT_FILE("rmf");
    process_options(argc, argv);

    RMF::FileConstHandle rh = RMF::open_rmf_file_read_only(input);
    RMF::show_frames(rh, std::cout);
    return 0;
  }
  catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}
