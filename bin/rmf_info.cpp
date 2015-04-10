/**
 * Copyright 2007-2015 IMP Inventors. All rights reserved.
 */

#include <exception>
#include <iostream>
#include <sstream>
#include <string>

#include "RMF/FileConstHandle.h"
#include "RMF/ID.h"
#include "RMF/info.h"
#include "RMF/decorator/alternatives.h"
#include "common.h"

namespace {
std::string description("Print out information about categories and keys.");

unsigned int frame = 0;
}

int main(int argc, char** argv) {
  try {
    options.add_options()("frame,f",
                          boost::program_options::value<unsigned int>(&frame),
                          "Frame to use");
    RMF_ADD_INPUT_FILE("rmf");
    process_options(argc, argv);

    RMF::FileConstHandle rh = RMF::open_rmf_file_read_only(input);
    if (!rh.get_description().empty()) {
      std::cout << "description: " << rh.get_description() << std::endl;
    }
    if (!rh.get_producer().empty()) {
      std::cout << "producer: " << rh.get_producer() << std::endl;
    }
    rh.set_current_frame(RMF::FrameID(frame));
    std::cout << "frames: " << rh.get_number_of_frames() << std::endl;
    RMF::show_info(rh, std::cout);
    RMF::decorator::AlternativesFactory af(rh);
    using RMF::operator<<;
    std::cout << "resolutions: " << RMF::decorator::get_resolutions(
                                        rh.get_root_node(), RMF::PARTICLE, 0)
              << std::endl;
    std::cout << "gaussian resolutions: "
              << RMF::decorator::get_resolutions(rh.get_root_node(),
                                                 RMF::GAUSSIAN_PARTICLE, 0)
              << std::endl;
    return 0;
  }
  catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}
