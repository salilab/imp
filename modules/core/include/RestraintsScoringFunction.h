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
#include <IMP/kernel/ScoringFunction.h>
#include <IMP/kernel/Restraint.h>
#include <IMP/kernel/internal/RestraintsScoringFunction.h>

IMPCORE_BEGIN_NAMESPACE

/** Create a scoring function on a list of restraints.
*/
class RestraintsScoringFunction :
#if defined(IMP_DOXYGEN) || defined(SWIG)
  public kernel::ScoringFunction
#else
    public IMP::kernel::internal::RestraintsScoringFunction
#endif
    {
 public:
  RestraintsScoringFunction(const kernel::RestraintsAdaptor &rs,
                            double weight = 1.0,
                            double max = kernel::NO_MAX,
                            std::string name = "RestraintsScoringFunction%1%")
    : kernel::internal::RestraintsScoringFunction(rs, weight, max, name) {}
  RestraintsScoringFunction(const kernel::RestraintsAdaptor &rs,
                            std::string name)
    : kernel::internal::RestraintsScoringFunction(rs, 1.0,
                                                  kernel::NO_MAX, name) {}
#if defined(SWIG)
      void do_add_score_and_derivatives(kernel::ScoreAccumulator sa,
                             const kernel::ScoreStatesTemp &ss) IMP_OVERRIDE;
  virtual kernel::Restraints create_restraints() const IMP_OVERRIDE;
      virtual kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(RestraintsScoringFunction);
#endif
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_RESTRAINTS_SCORING_FUNCTION_H */
