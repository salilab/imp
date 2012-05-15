/**
 * Copyright 2007-2012 IMP Inventors. All rights reserved.
 */
#include <IMP/rmf/atom_io.h>
#include <RMF/FileHandle.h>
#include <IMP/rmf/particle_io.h>
#include <IMP/display/declare_Geometry.h>
#include <IMP/display/Writer.h>
#include <IMP/display/restraint_geometry.h>
#include <IMP/rmf/geometry_io.h>
#include <IMP/atom/hierarchy_tools.h>
#include <IMP/rmf/restraint_io.h>
#include <IMP/rmf/frames.h>
#include "common.h"

std::string
description("Slice an rmf file into one or more files based on frames");


int main(int argc, char **argv) {
  try {
    IMP_ADD_INPUT_FILE("rmf");
    IMP_ADD_OUTPUT_FILE("rmf");
    IMP_ADD_FRAMES;
    process_options(argc, argv);

    IMP::set_log_level(IMP::SILENT);
    RMF::FileConstHandle rh= RMF::open_rmf_file_read_only(input);
    RMF::FileHandle orh= RMF::create_rmf_file(output);
    RMF::copy_structure(rh, orh);

    IMP_FOR_EACH_FRAME(rh.get_number_of_frames()) {
      RMF::copy_frame(rh, orh, current_frame, frame_iteration);
    }
    return 0;
  } catch (const IMP::Exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}
