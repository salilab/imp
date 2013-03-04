/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <RMF/FileConstHandle.h>
#include <RMF/utility.h>
#include "common.h"
#include <sstream>

namespace {
std::string description
  = "Validate a saved RMF file.";
}



int main(int argc, char **argv) {
  try {
    RMF_ADD_INPUT_FILE("rmf");
    process_options(argc, argv);
    RMF::FileConstHandle rh = RMF::open_rmf_file_read_only(input);
    rh.validate(std::cerr);
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}
