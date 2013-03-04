/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <RMF/FileHandle.h>
#include <RMF/utility.h>
#include "common.h"

namespace {
  std::vector<std::string> inputs;
  std::string description("Combine two or more rmf files.");
  std::string output;
}
int main(int argc, char **argv) {
  try {
    positional_options.add_options()
      ("input-files,i",
      boost::program_options::value< std::vector<std::string> >(&inputs),
      "input rmf file");
    positional_names.push_back("input_1.rmf input_2.rmf ... output.rmf");
    positional_options_description.add("input-files", -1);
    process_options(argc, argv);
    if (inputs.size() < 3) {
      print_help_and_exit(argv);
    }

    output = inputs.back();
    inputs.pop_back();
    RMF::FileConstHandle rh = RMF::open_rmf_file_read_only(inputs[0]);
    RMF::FileHandle orh = RMF::create_rmf_file(output);
    RMF::copy_structure(rh, orh);
    int out_frame = 0;
    for (unsigned int i = 0; i < inputs.size(); ++i) {
      // avoid double open
      if (i != 0) {
        rh = RMF::open_rmf_file_read_only(inputs[i]);
        RMF::link_structure(rh, orh);
      }

      for (unsigned int j = 0; j < rh.get_number_of_frames(); ++j) {
        rh.set_current_frame(j);
        orh.set_current_frame(out_frame);
        RMF::copy_frame(rh, orh);
        ++out_frame;
      }
    }
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}
