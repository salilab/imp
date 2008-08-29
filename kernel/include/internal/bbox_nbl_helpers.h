/**
 *  \file bbox_nbl_helpers.h
 *  \brief Helpers for the bbox NBL.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_BBOX_NBL_HELPERS_H
#define __IMP_BBOX_NBL_HELPERS_H

#include "../base_types.h"
#include "../Particle.h"

namespace IMP
{

namespace internal
{


// defined in NonbondedListScoreState.h
class NBLAddPairIfNonbonded;

/** Scan for overlapping bounding boxes within the set of particles with
    radii. The radii are assume to be 0 if missing. Slack and cutoff/2 are
    added to the radii. ap(Paritcle*, Particle*) is called for each
    intersecting pair */
void bbox_scan(const Particles &ps, FloatKey rk, Float slack, Float cutoff,
               const NBLAddPairIfNonbonded &ap);

/** Bipartite version of above */
void bipartite_bbox_scan(const Particles &ps0, const Particles &ps1,
                         FloatKey rk, Float slack, Float cutoff,
                         const NBLAddPairIfNonbonded &ap);

} // namespace internal

} // namespace IMP

#endif  /* __IMP_BBOX_NBL_HELPERS_H */
