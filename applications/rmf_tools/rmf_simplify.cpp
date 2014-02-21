/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */
#include <IMP/atom/hierarchy_tools.h>
#include <IMP/atom/Chain.h>
#include <IMP/atom/pdb.h>
#include <IMP/base/flags.h>
#include <IMP/base/Flag.h>
#include <IMP/rmf/atom_io.h>
#include <IMP/atom/force_fields.h>
#include <IMP/atom/Representation.h>
#include <IMP/rmf/frames.h>
#include <RMF/FileHandle.h>

int main(int argc, char *argv[]) {
  IMP::base::Flag<double> resolution("resolution", "The resolution to use.",
                                     1.0);
  IMP::base::Flag<bool> multiresolution(
      "multiresolution", "Whether to create a multiresolution representation.");

  IMP::Strings args = IMP::base::setup_from_argv(
      argc, argv, "Create a simplified representation of a pdb",
      "input.pdb output.rmf", 2);

  IMP_NEW(IMP::kernel::Model, m, ());
  IMP::atom::Hierarchy hr = IMP::atom::read_pdb(args[0], m);
  if (multiresolution) {
    IMP::atom::add_bonds(hr);
  }

  IMP_FOREACH(IMP::atom::Hierarchy c,
              IMP::atom::get_by_type(hr, IMP::atom::CHAIN_TYPE)) {
    IMP::atom::Hierarchy cur =
        create_simplified_assembly_from_volume(hr, resolution);
    if (multiresolution) {
      IMP::atom::Representation rep =
          IMP::atom::Representation::setup_particle(c);
      rep.add_representation(cur);
    } else {
      IMP::atom::Hierarchy p = c.get_parent();
      p.remove_child(c);
      c.add_child(cur);
    }
  }

  RMF::FileHandle fh = RMF::create_rmf_file(args[1]);
  IMP::rmf::add_hierarchy(fh, hr);
  IMP::rmf::save_frame(fh, "frame");
  return 0;
}
