/**
 * \file IMP/atom/force_fields.h
 * \brief Define functions to add bonds and radii to atoms.
 *
 * Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_FORCE_FIELDS_H
#define IMPATOM_FORCE_FIELDS_H

#include <IMP/atom/atom_config.h>
#include "Hierarchy.h"
#include "ForceFieldParameters.h"
#include "CHARMMParameters.h"

#include <string>

IMPATOM_BEGIN_NAMESPACE

/**
   Add bonds using definitions from given force field parameters. Note
   that, at the moment, all added bonds are reported as
   IMP::Bond::SINGLE, whether or not they actually are.

   \see Hierarchy
   \see ForceFieldParameters
*/
IMPATOMEXPORT void add_bonds(Hierarchy d, const ForceFieldParameters* ffp =
                                              get_all_atom_CHARMM_parameters());

/**
   Add vdW radius from given force field.

   \see Hierarchy
   \see ForceFieldParameters
*/
IMPATOMEXPORT void add_radii(Hierarchy d, const ForceFieldParameters* ffp =
                                              get_all_atom_CHARMM_parameters(),
                             FloatKey radius_key = FloatKey("radius"));

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_FORCE_FIELDS_H */
