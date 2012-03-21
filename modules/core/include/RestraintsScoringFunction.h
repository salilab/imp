/**
 *  \file IMP/RestraintsScoringFunction.h
 *  \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_RESTRAINTS_SCORING_FUNCTION_H
#define IMPCORE_RESTRAINTS_SCORING_FUNCTION_H

#include "core_config.h"
#include <IMP/ScoringFunction.h>
#include <IMP/internal/RestraintsScoringFunction.h>


IMPCORE_BEGIN_NAMESPACE

/** Create a scoring function on a list of restraints.
*/
class IMPEXPORT RestraintsScoringFunction:
#if defined(IMP_DOXYGEN) || defined(SWIG)
public ScoringFunction
#else
public IMP::internal::RestraintsScoringFunction
#endif
 {
 public:
  RestraintsScoringFunction(const RestraintsTemp &rs,
                            double weight=1.0,
                            double max=NO_MAX,
                            std::string name= "RestraintsScoringFunction%1%");
  IMP_SCORING_FUNCTION(RestraintsScoringFunction);
};


IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_RESTRAINTS_SCORING_FUNCTION_H */
