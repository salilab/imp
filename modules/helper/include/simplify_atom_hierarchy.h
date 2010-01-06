/**
 *  \file helper/simplify_atom_hierarchy.h
 *  \brief functionality for defining rigid bodies
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
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

    \unstable{create_simplified}
 */
IMPHELPEREXPORT atom::Hierarchy create_simplified(atom::Hierarchy in,
                                                  double resolution);

/** Produce a coarser molecule by covering every consecutive num_res with
   a sphere. The mass of each new sphere is the sum of the masses of
   its covered particles.
   In case the input Hierarchy consist of multiple chains,
   spheres will not cover atoms of different chains.
    \unstable{create_simplified_by_residue}
 */
IMPHELPEREXPORT atom::Hierarchy create_simplified_by_residue(atom::Hierarchy in,
                                                  int num_res);

/** Produce a coarser molecule by covering every input residue segment
    with a sphere. The mass of each new sphere is the sum of the masses of
   its covered particles.
   \note The function supports a single chain
    \unstable{create_simplified_by_by_segments}
 */
typedef std::pair<int,int> ResidueIndexPair;
typedef std::vector<ResidueIndexPair> ResidueIndexPairVec;

IMPHELPEREXPORT atom::Hierarchy create_simplified_by_segments(
           atom::Hierarchy in,
           const ResidueIndexPairVec residue_segments);

IMPHELPER_END_NAMESPACE

#endif  /* IMPHELPER_HELPER_SIMPLIFY_ATOM_HIERARCHY_H */
