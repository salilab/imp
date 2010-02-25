/**
 * \file force_fields \brief
 *
 * Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include <IMP/atom/force_fields.h>
#include <IMP/atom/CharmmParameters.h>
#include <IMP/atom/charmm_topology.h>

IMPATOM_BEGIN_NAMESPACE


void add_bonds(Hierarchy d, const ForceFieldParameters* ffp)
{
  ffp->add_bonds(d);
}



void add_radii(Hierarchy d, const ForceFieldParameters* ffp,
                FloatKey radius_key)
{
  // Temporary hack to maintain old interface for SAXS
  const CharmmParameters *cp = dynamic_cast<const CharmmParameters *>(ffp);
  if (cp) {
    IMP::Pointer<CHARMMTopology> top = cp->make_topology(d);
    top->apply_default_patches(cp);
    top->add_atom_types(d);
  }
  ffp->add_radii(d, radius_key);
}

IMPATOM_END_NAMESPACE
