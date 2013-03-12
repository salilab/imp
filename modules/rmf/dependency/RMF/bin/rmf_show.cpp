/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <RMF/FileHandle.h>
#include <RMF/NodeConstHandle.h>
#include "common.h"
namespace {
std::string description("Show an rmf file as text.");
}
int main(int argc, char **argv) {
  try {
    options.add_options()
      ("decorators,d", "Show what decorators recognize each node.");
    RMF_ADD_INPUT_FILE("rmf");
    int frame = 0;
    options.add_options() ("frame,f",
                           boost::program_options::value< int >(&frame),
                           "Frame to use, if -1 show static data");

    boost::program_options::variables_map vm(process_options(argc, argv));
    RMF::FileConstHandle rh = RMF::open_rmf_file_read_only(input);
    std::string descr = rh.get_description();
    if (!descr.empty()) {
      std::cout << descr << std::endl;
    }
    std::string prod = rh.get_producer();
    if (!prod.empty()) {
      std::cout << prod << std::endl;
    }
    rh.set_current_frame(frame);
    if (vm.count("decorators")) {
      RMF::show_hierarchy_with_decorators(rh.get_root_node(),
                                          vm.count("verbose"),
                                          std::cout);
    } else if (vm.count("verbose") == 0) {
      RMF::show_hierarchy(rh.get_root_node(), std::cout);
    } else {
      RMF::show_hierarchy_with_values(rh.get_root_node(),
                                      std::cout);
    }
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
  return 0;
}
