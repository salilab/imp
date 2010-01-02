/**
 *  \file helper/optimization.h
 *  \brief functionality for defining rigid bodies
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPHELPER_HELPER_OPTIMIZATION_H
#define IMPHELPER_HELPER_OPTIMIZATION_H

#include "config.h"
#include <IMP/core/XYZR.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/OptimizerState.h>


IMPHELPER_BEGIN_NAMESPACE

/** Try to find one conformation of the spheres with a low score.
    \param[in] m The model defining the scoring function
    and the set of optimized spheres.
    \param[in] bb The bounding box for the solution.
    \param[in] cutoff Look for a solution scoring less than cutoff.
    \param[in] oss A set of optimizer states to add to optimizers.

    \throw IMP::UsageException if something other than the cartesian
    coordinates or radius needs to be optimized.

    \note This method is currently very simple, but will become increasingly
    sophisticated with time.

    \unstable{optimize_spheres}
    \untested{optimize_spheres}
*/
IMPHELPEREXPORT double
optimize_spheres(Model *m, const algebra::BoundingBox3D &bb,
                 double cutoff= std::numeric_limits<double>::max(),
                 const OptimizerStates &oss= OptimizerStates());



IMPHELPER_END_NAMESPACE

#endif  /* IMPHELPER_HELPER_OPTIMIZATION_H */
