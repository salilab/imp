/**
 * Copyright 2007-2012 IMP Inventors. All rights reserved.
 */
#include <RMF/FileHandle.h>
#include <sstream>
#include "common.h"

std::string description("Lock an rmf file.");

int frame=0;

int main(int argc, char **argv) {
  try {
    IMP_ADD_INPUT_FILE("rmf");
    process_options(argc, argv);


    RMF::FileConstHandle rh= RMF::open_rmf_file_read_only(input);
    rh.set_is_locked(true);
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}
