/**
 *  \file IMP/declare_ScoringFunction.h
 *  \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INTERNAL_RESTRAINTS_SCORING_FUNCTION_H
#define IMPKERNEL_INTERNAL_RESTRAINTS_SCORING_FUNCTION_H

#include "../kernel_config.h"
#include "../ScoringFunction.h"
#include "../scoring_function_macros.h"

IMP_BEGIN_NAMESPACE
class Model;
IMP_END_NAMESPACE

IMP_BEGIN_INTERNAL_NAMESPACE

/** Create a scoring function on a list of restraints.
*/
class IMPEXPORT RestraintsScoringFunction: public ScoringFunction {
  friend class Model;
  Restraints rs_;
  RestraintSets rss_;
  double weight_;
  double max_;
protected:
  // Does not reset dependencies
  void set_restraints(const RestraintsTemp &rs);
  RestraintsScoringFunction(Model *m,
                            double weight=1.0,
                            double max=NO_MAX,
                            std::string name= "RestraintsScoringFunction%1%");
 public:
  RestraintsScoringFunction(const RestraintsTemp &rs,
                            double weight=1.0,
                            double max=NO_MAX,
                            std::string name= "RestraintsScoringFunction%1%");

  RestraintsTemp get_restraints() const {
    return static_cast<RestraintsTemp>(rs_)+static_cast<RestraintsTemp>(rss_);
  }
  IMP_SCORING_FUNCTION(RestraintsScoringFunction);
};
IMP_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_RESTRAINTS_SCORING_FUNCTION_H */
