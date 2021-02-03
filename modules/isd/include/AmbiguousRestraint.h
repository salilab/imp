/**
 *  \file IMP/isd/AmbiguousRestraint.h
 *  \brief An implementation of the d-norm to make an ambiguous restraint.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_AMBIGUOUS_RESTRAINT_H
#define IMPISD_AMBIGUOUS_RESTRAINT_H

#include <IMP/isd/isd_config.h>
#include <IMP/Restraint.h>
#include <IMP/Particle.h>
#include <IMP/generic.h>

IMPISD_BEGIN_NAMESPACE

//! Apply an ambiguous restraint by computing the d-norm.
//  d is an integer.
//  If d>0, behaves like an "and" function.
//  If d<0, behaves like an "or" function.
//  Limits are fuzzy "and" (min) and fuzzy "or" (max) when d is infinite.
/*
    The source code is as follows:
    \include AmbiguousRestraint.h
    \include AmbiguousRestraint.cpp
*/
class IMPISDEXPORT AmbiguousRestraint : public Restraint {
  int d_;
  Restraints rs_;

 public:
  //! Create the restraint.
  /** Two ways to call it: pass it two restraints, or a list of restraints.
   */
  AmbiguousRestraint(Model *m, int d, Restraint *r0,
                     Restraint *r1);
  AmbiguousRestraint(Model *m, int d, Restraints rs);

  double get_probability() const { return exp(-unprotected_evaluate(nullptr)); }

  virtual double unprotected_evaluate(IMP::DerivativeAccumulator *accum)
      const IMP_OVERRIDE;
  virtual IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(AmbiguousRestraint);
};

IMPISD_END_NAMESPACE

#endif /* IMPISD_AMBIGUOUS_RESTRAINT_H */
