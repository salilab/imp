/**
 *  \file helper/simplify_atom_hierarchy.h
 *  \brief functionality for defining rigid bodies
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPHELPER_HELPER_SIMPLIFY_ATOM_HIERARCHY_H
#define IMPHELPER_HELPER_SIMPLIFY_ATOM_HIERARCHY_H

#include "config.h"
#include <IMP/core/XYZR.h>
#include <IMP/atom/Hierarchy.h>


IMPHELPER_BEGIN_NAMESPACE



/** Produce a new atom::Hierarchy which is a simpler copy of the input.
    The number of balls used is guessed from the resolution. Each
    a Residue corresponding to each Residue of the input is added as
    a particle without coordinates to each of the representational
    spheres of the output hierarchy.


    \unstable(get_simplified_2)
 */
IMPHELPEREXPORT atom::Hierarchy create_simplified_2(atom::Hierarchy in,
                                                 double resolution);





IMPHELPER_END_NAMESPACE

#endif  /* IMPHELPER_HELPER_SIMPLIFY_ATOM_HIERARCHY_H */
