/**
 *  \file IMP/rmf/internal/hierarchy_links_helpers.cpp
 *  \brief
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 */

#include <IMP/rmf/internal/hierarchy_links_helpers.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/Residue.h>
#include <IMP/check_macros.h>
#include <sstream>

IMPRMF_BEGIN_INTERNAL_NAMESPACE


std::string get_good_name_to_atom_node(Model *m, ParticleIndex h) {
  if (atom::Atom::get_is_setup(m, h)) {
    return atom::Atom(m, h).get_atom_type().get_string();
  } else if (atom::Residue::get_is_setup(m, h)) {
    std::ostringstream oss;
    oss << atom::Residue(m, h).get_index();
    return oss.str();
  } else {
    return m->get_particle_name(h);
  }
}


// Get the 'child' provenance from the RMF, or a default-constructed
// object if no children exist
RMF::NodeConstHandle get_previous_rmf_provenance(RMF::NodeConstHandle node) {
  RMF::NodeConstHandles nchs;
  IMP_FOREACH(RMF::NodeConstHandle ch, node.get_children()) {
    if (ch.get_type() == RMF::PROVENANCE) {
      nchs.push_back(ch);
    }
  }
  if (nchs.size() > 1) {
    IMP_THROW("RMF provenance hierarchy has more than one child at " << node,
              IOException);
  } else if (nchs.empty()) {
    return RMF::NodeConstHandle();
  } else {
    return nchs[0];
  }
}





IMPRMF_END_INTERNAL_NAMESPACE
