/**
 *  \file IMP/core/blame.h    \brief Various important functionality
 *                                       for implementing decorators.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_BLAME_H
#define IMPCORE_BLAME_H

#include <IMP/core/core_config.h>
#include <IMP/display/declare_Geometry.h>
#include <IMP/constants.h>
#include <IMP/base_types.h>

IMPCORE_BEGIN_NAMESPACE

/** Assign blame to the passed particles by dividing up the scores of the
    passed restraints over the particles they add on. The restraints will
    be decomposed first in order to make sure that the assignment is as
    fine grained as possible.

    The main cost is evaluating the restraints.
*/
IMPCOREEXPORT void assign_blame(const kernel::RestraintsTemp &rs,
                                const ParticlesTemp &ps, FloatKey attribute);

/** Create a set of geometry objects colored by the blame as described in
    the docs of the assign_blame() function.

    All particles must be XYZR particles.

    If max is NO_MAX, then the max is found automatically.
*/
IMPCOREEXPORT display::Geometries create_blame_geometries(
    const kernel::RestraintsTemp &rs, const ParticlesTemp &ps,
    double max = NO_MAX,
    std::string name = std::string());

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_BLAME_H */
