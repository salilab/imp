/**
 * Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "RMF/FileConstHandle.h"
#include "RMF/FileHandle.h"
#include "RMF/ID.h"
#include "RMF/infrastructure_macros.h"
#include "RMF/utility.h"
#include "common.h"

namespace {
std::vector<std::string> inputs;
std::string description("Combine two or more rmf files.");
std::string output;
}
int main(int argc, char** argv) {
  try {
    positional_options.add_options()(
        "input-files,i",
        boost::program_options::value<std::vector<std::string> >(&inputs),
        "input rmf file");
    positional_names.push_back("input_1.rmf input_2.rmf ... output.rmf");
    positional_options_description.add("input-files", -1);
    process_options(argc, argv);
    if (inputs.size() < 3) {
      print_help_and_exit(argv);
    }

    output = inputs.back();
    inputs.pop_back();
    RMF::FileHandle orh = RMF::create_rmf_file(output);
    orh.set_producer("rmf_cat");
    for (unsigned int i = 0; i < inputs.size(); ++i) {
      RMF::FileConstHandle rh = RMF::open_rmf_file_read_only(inputs[i]);
      if (i == 0) {
        RMF::clone_file_info(rh, orh); // creator etc. (not essential)
        RMF::clone_hierarchy(rh, orh);
        RMF::clone_static_frame(rh, orh);
      }
      orh.set_description(orh.get_description() + "\n" + rh.get_description());
      for(RMF::FrameID ni : rh.get_frames()) {
        rh.set_current_frame(ni);
        orh.add_frame(rh.get_name(ni), rh.get_type(ni));
        RMF::clone_loaded_frame(rh, orh);
      }
    }
    return 0;
  }
  catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}
