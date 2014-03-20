/**
 *  \file IMP/isd/AmbiguousRestraint.h
 *  \brief An implementation of the d-norm to make an ambiguous restraint.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_AMBIGUOUS_RESTRAINT_H
#define IMPISD_AMBIGUOUS_RESTRAINT_H

#include <IMP/isd/isd_config.h>
#include <IMP/kernel/Restraint.h>

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
class IMPISDEXPORT AmbiguousRestraint : public kernel::Restraint {
  int d_;
  kernel::Restraints rs_;

 public:
  //! Create the restraint.
  /** kernel::Restraints should store the particles they are to act on,
      preferably in a Singleton or PairContainer as appropriate.
      Two ways to call it: pass it two restraints, or a list of restraints.
   */
  AmbiguousRestraint(kernel::Model *m, int d, kernel::Restraint *r0,
                     kernel::Restraint *r1);
  AmbiguousRestraint(kernel::Model *m, int d, kernel::Restraints rs);

  double get_probability() const { return exp(-unprotected_evaluate(nullptr)); }

  /** This macro declares the basic needed methods: evaluate and show
   */
  virtual double unprotected_evaluate(IMP::kernel::DerivativeAccumulator *accum)
      const IMP_OVERRIDE;
  virtual IMP::kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(AmbiguousRestraint);
};

IMPISD_END_NAMESPACE

#endif /* IMPISD_AMBIGUOUS_RESTRAINT_H */
