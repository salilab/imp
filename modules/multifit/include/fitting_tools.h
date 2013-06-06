/**
 *  \file IMP/multifit/fitting_tools.h
 *  \brief tools for handling fitting records
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_FITTING_TOOLS_H
#define IMPMULTIFIT_FITTING_TOOLS_H

#include <IMP/algebra/Transformation3D.h>
#include <IMP/base_types.h>
#include <IMP/core/Hierarchy.h>
#include <IMP/base/Object.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/multifit/multifit_config.h>
#include "FittingSolutionRecord.h"
IMPMULTIFIT_BEGIN_NAMESPACE

//! prune solutions by distance to an anchor point
/**
\param[in] fit_sols initial fitting solutions
\param[in] mh the molecule the fitting solutions apply for
\param[in] ap anchor point for which the transformed mh (fit) should be
              close to
\param[in] dist all fits such that the distance between ap and the fit center
                is smaller than dist will be included
\return the pruned fitting solutions
 */
IMPMULTIFITEXPORT FittingSolutionRecords get_close_to_point(
  const FittingSolutionRecords &fit_sols,
  atom::Hierarchy mh,
  IMP::Particle *ap,
  Float dist);

IMPMULTIFIT_END_NAMESPACE
#endif /* IMPMULTIFIT_FITTING_TOOLS_H */
