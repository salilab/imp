/**
 *  \file helper/atom_hierarchy.h
 *  \brief functionality for defining rigid bodies
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPHELPER_HELPER_ATOM_HIERARCHY_H
#define IMPHELPER_HELPER_ATOM_HIERARCHY_H

#include "config.h"
#include <IMP/core/XYZR.h>
#include <IMP/atom/Hierarchy.h>


IMPHELPER_BEGIN_NAMESPACE

//! Create a coarse grained molecule
/** The coarse grained model is created with a number of spheres
    based on the resolution and the volume. If the volume is not provided
    it is estimated based on the number of residues. The protein is
    created as a molecular hierarchy rooted at p. The leaves are Domain
    particles wtih appropriate residue indexes stored and are XYZR
    particles.

    Volume is, as usual, in cubic anstroms.

    Currently the function creates a set of balls with radii no greater
    than resolution which overlap by 20% and have a volume of their
    union equal to the passed volume. The balls are held together by
    a ConnectivityRestraint with the given spring constant.

    The coordinates of the balls defining the protein are optimized
    by default, and have garbage values.

    \return A restraint which will enforce appropriate connectivity
    of the protein.
    \untested{create_protein}
    \unstable{create_protein}
    \relatesalso atom::Hierarchy
 */
IMPHELPEREXPORT Restraint* create_protein(Particle *p,
                                        double resolution,
                                        int number_of_residues,
                                        int first_residue_index=0,
                                        double volume=-1,
                                        double spring_strength=1);




IMPHELPER_END_NAMESPACE

#endif  /* IMPHELPER_HELPER_ATOM_HIERARCHY_H */
