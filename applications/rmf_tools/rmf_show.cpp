/**
 * Copyright 2007-2012 IMP Inventors. All rights reserved.
 */
#include <IMP/rmf/atom_io.h>
#include <RMF/FileHandle.h>
#include <IMP/rmf/geometry_io.h>

#include "common.h"

std::string description("Show an rmf file as text.");
int frame=0;
int main(int argc, char **argv) {
  try {
    options.add_options()
      ("decorators,d", "Show what decorators recognize each node.");
    IMP_ADD_INPUT_FILE("rmf");
    IMP_ADD_FRAMES;
    boost::program_options::variables_map vm(process_options(argc, argv));
    RMF::FileConstHandle rh= RMF::open_rmf_file_read_only(input);
    std::string descr= rh.get_description();
    if (!descr.empty()) {
      std::cout << descr << std::endl;
    }
    if (vm.count("decorators")) {
      RMF::show_hierarchy_with_decorators(rh.get_root_node(),
                                          vm.count("verbose"),
                                          frame, std::cout);
    } else if (vm.count("verbose")==0) {
      RMF::show_hierarchy(rh.get_root_node(), std::cout);
    } else {
      if (frame_option>=0) {
        begin_frame=frame_option;
        end_frame=-1;
        frame_step=-1;
      } else {
        begin_frame=0;
        end_frame= rh.get_number_of_frames();
        frame_step=-frame_option;
      }
      RMF::show_hierarchy_with_values(rh.get_root_node(),
                                      begin_frame, end_frame, frame_step,
                                      std::cout);
    }
  } catch (const IMP::base::Exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
  return 0;
}
