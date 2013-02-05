/**
 *  \file IMP/declare_ScoringFunction.h
 *  \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INTERNAL_RESTRAINTS_SCORING_FUNCTION_H
#define IMPKERNEL_INTERNAL_RESTRAINTS_SCORING_FUNCTION_H

#include <IMP/kernel/kernel_config.h>
#include "../ScoringFunction.h"
#include "../scoring_function_macros.h"
#include "../container_macros.h"

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

/** Create a scoring function on a list of restraints.
*/
class IMPKERNELEXPORT RestraintsScoringFunction: public ScoringFunction {
  double weight_;
  double max_;
protected:
  RestraintsScoringFunction(Model *m,
                            double weight=1.0,
                            double max=NO_MAX,
                            std::string name= "RestraintsScoringFunction%1%");
 public:
  RestraintsScoringFunction(const RestraintsTemp &rs,
                            double weight=1.0,
                            double max=NO_MAX,
                            std::string name= "RestraintsScoringFunction%1%");

  IMP_LIST(public, Restraint, restraint, Restraint*, Restraints);

  void do_add_score_and_derivatives(IMP::kernel::ScoreAccumulator sa,
                                    const ScoreStatesTemp &ss) IMP_OVERRIDE;
  Restraints create_restraints() const IMP_OVERRIDE;
  ScoreStatesTemp get_required_score_states() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(RestraintsScoringFunction);

};
IMPKERNEL_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_RESTRAINTS_SCORING_FUNCTION_H */
