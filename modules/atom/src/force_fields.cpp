/**
 * \file force_fields \brief
 *
 * Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/atom/force_fields.h>

IMPATOM_BEGIN_NAMESPACE


void add_bonds(Hierarchy d, const ForceFieldParameters* ffp)
{
  ffp->add_bonds(d);
}



void add_radii(Hierarchy d, const ForceFieldParameters* ffp,
                FloatKey radius_key)
{
  ffp->add_radii(d, radius_key);
}

IMPATOM_END_NAMESPACE
