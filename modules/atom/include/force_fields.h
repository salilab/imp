/**
 * \file force_fields.h \brief
 *
 * Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */
#ifndef IMPATOM_FORCE_FIELDS_H
#define IMPATOM_FORCE_FIELDS_H

#include "config.h"
#include "Hierarchy.h"
#include "ForceFieldParameters.h"

#include <string>

IMPATOM_BEGIN_NAMESPACE

/**
   Add bonds using definitions from given force field parameters
*/
IMPATOMEXPORT void add_bonds(Hierarchy d,
                             const ForceFieldParameters* ffp
                             =default_force_field_parameters());


/**
   Add vdW radius from from given force field
*/
IMPATOMEXPORT void add_radii(Hierarchy d,
                             const ForceFieldParameters *ffp
                             = default_force_field_parameters(),
                             FloatKey radius_key= FloatKey("radius"));

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_FORCE_FIELDS_H */
