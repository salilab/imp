/**
 * Copyright 2007-2019 IMP Inventors. All rights reserved.
 */

#include <exception>
#include <iostream>
#include <string>

#include "RMF/FileConstHandle.h"
#include "RMF/FileHandle.h"
#include "RMF/ID.h"
#include "RMF/infrastructure_macros.h"
#include "RMF/utility.h"
#include "common.h"

namespace {
std::string description("Grab frames from an rmf file");
}

int main(int argc, char** argv) {
  try {
    RMF_ADD_INPUT_FILE("input_rmf");
    RMF_ADD_OUTPUT_FILE("output_rmf");
    RMF_ADD_FRAMES;
    bool quiet=false;
    options.add_options()("quiet,q",
                          boost::program_options::bool_switch(&quiet),
                          "Suppress all non-error output");
    process_options(argc, argv);

    RMF::FileConstHandle rh = RMF::open_rmf_file_read_only(input);
    RMF::FileHandle orh = RMF::create_rmf_file(output);
    RMF::clone_file_info(rh, orh);
    orh.set_producer("rmf_slice");
    RMF::clone_hierarchy(rh, orh);
    RMF::clone_static_frame(rh, orh);
    if(!quiet) std::cout << "Copying frames";
    unsigned num_copied = 0;
    RMF_FOREACH(RMF::FrameID f, rh.get_frames()) {
      if (f.get_index() < static_cast<unsigned int>(start_frame)) continue;
      if ((f.get_index() - start_frame) % step_frame != 0) continue;
      rh.set_current_frame(f);
      orh.add_frame(rh.get_name(f), rh.get_type(f));
      RMF::clone_loaded_frame(rh, orh);
      num_copied++;
      if (orh.get_number_of_frames() % 10 == 0) std::cout << "." << std::flush;
    }
    if(!quiet) std::cout << std::endl;
    if(!quiet) std::cout << num_copied << " frames copied" << std::endl;
    return 0;
  }
  catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
