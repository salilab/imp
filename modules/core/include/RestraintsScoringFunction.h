/**
 *  \file IMP/core/RestraintsScoringFunction.h
 *  \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_RESTRAINTS_SCORING_FUNCTION_H
#define IMPCORE_RESTRAINTS_SCORING_FUNCTION_H

#include <IMP/core/core_config.h>
#include <IMP/ScoringFunction.h>
#include <IMP/internal/RestraintsScoringFunction.h>


IMPCORE_BEGIN_NAMESPACE

/** Create a scoring function on a list of restraints.
*/
class RestraintsScoringFunction:
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
                            std::string name= "RestraintsScoringFunction%1%"):
      IMP::internal::RestraintsScoringFunction(rs, weight, max, name)
   {
   }
  RestraintsScoringFunction(const RestraintsTemp &rs,
                            std::string name):
    IMP::internal::RestraintsScoringFunction(rs, 1.0, NO_MAX, name)
   {
   }
#if defined(SWIG)
   IMP_SCORING_FUNCTION(RestraintsScoringFunction);
#endif
};


IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_RESTRAINTS_SCORING_FUNCTION_H */
