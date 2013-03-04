/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <RMF/FileHandle.h>
#include <sstream>
#include "common.h"

namespace {
std::string description("Print out information about frames");
}

int main(int argc, char **argv) {
  try {
    RMF_ADD_INPUT_FILE("rmf");
    process_options(argc, argv);


    RMF::FileConstHandle rh = RMF::open_rmf_file_read_only(input);
    for (unsigned int i = 0; i < rh.get_number_of_frames(); ++i) {
      rh.set_current_frame(i);
      std::string cmt = rh.get_current_frame().get_name();
      if (!cmt.empty()) {
        std::cout << i << ": " << cmt << std::endl;
      }
    }
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}
