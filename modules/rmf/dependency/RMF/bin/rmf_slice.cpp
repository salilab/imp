/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <RMF/FileHandle.h>
#include <RMF/utility.h>
#include "common.h"

namespace {
std::string
  description("Grab frames from an rmf file");
}

int main(int argc, char **argv) {
  try {
    RMF_ADD_INPUT_FILE("rmf");
    RMF_ADD_OUTPUT_FILE("rmf");
    RMF_ADD_FRAMES;
    process_options(argc, argv);

    RMF::FileConstHandle rh = RMF::open_rmf_file_read_only(input);
    RMF::FileHandle orh = RMF::create_rmf_file(output);
    RMF::copy_structure(rh, orh);
    rh.set_current_frame(RMF::ALL_FRAMES);
    orh.set_current_frame(RMF::ALL_FRAMES);
    RMF::copy_frame(rh, orh);
    RMF_FOR_EACH_FRAME(rh.get_number_of_frames()) {
      rh.set_current_frame(current_frame);
      orh.set_current_frame(frame_iteration);
      RMF::copy_frame(rh, orh);
    }
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}
