/**
 * Copyright 2007-2017 IMP Inventors. All rights reserved.
 */

#include <exception>
#include <iostream>
#include <sstream>
#include <string>

#include "RMF/FileConstHandle.h"
#include "RMF/ID.h"
#include "RMF/NodeConstHandle.h"
#include "RMF/signature.h"
#include "common.h"

namespace {
std::string description("Print the signature for an RMF file.");
}
int main(int argc, char** argv) {
  try {
    RMF_ADD_INPUT_FILE("rmf");

    boost::program_options::variables_map vm(process_options(argc, argv));
    RMF::FileConstHandle rh = RMF::open_rmf_file_read_only(input);
    std::cout << RMF::get_signature_string(rh);
  }
  catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
  return 0;
}
