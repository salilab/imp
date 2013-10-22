/**
 * \file force_fields \brief
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/force_fields.h>
#include <IMP/atom/CHARMMParameters.h>
#include <IMP/atom/charmm_segment_topology.h>

IMPATOM_BEGIN_NAMESPACE

void add_bonds(Hierarchy d, const ForceFieldParameters* ffp) {
  ffp->add_bonds(d);
}

void add_radii(Hierarchy d, const ForceFieldParameters* ffp,
               FloatKey radius_key) {
  // Temporary hack to maintain old interface for SAXS
  const CHARMMParameters* cp = dynamic_cast<const CHARMMParameters*>(ffp);
  if (cp) {
    base::Pointer<CHARMMTopology> top = cp->create_topology(d);
    top->apply_default_patches();
    top->add_atom_types(d);
  }
  ffp->add_radii(d, 1.0, radius_key);
}

IMPATOM_END_NAMESPACE
