/**
 * Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#include <IMP/rmf/atom_io.h>
#include <RMF/FileHandle.h>
#include <IMP/flags.h>
#include <IMP/atom/pdb.h>
#include <IMP/rmf/frames.h>
#include <IMP/atom/force_fields.h>

int main(int argc, char **argv) {
  try {
    IMP::Strings files = IMP::setup_from_argv(
        argc, argv, "Make an RMF file from a PDB.", "input.pdb output.rmf", 2);
    IMP_NEW(IMP::Model, m, ());
    IMP::atom::Hierarchy h = IMP::atom::read_pdb(files[0], m);
    IMP::atom::add_bonds(h);
    RMF::FileHandle out = RMF::create_rmf_file(files[1]);

    IMP::rmf::add_hierarchy(out, h);
    IMP::rmf::save_frame(out, "frame");
  }
  catch (const IMP::Exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
  catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
