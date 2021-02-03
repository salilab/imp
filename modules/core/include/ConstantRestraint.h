/**
 *  \file IMP/core/ConstantRestraint.h    \brief Constant restraint.
 *
 *  Just return a constant.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_CONSTANT_RESTRAINT_H
#define IMPCORE_CONSTANT_RESTRAINT_H

#include <IMP/core/core_config.h>

#include <IMP/Restraint.h>
#include <IMP/PairScore.h>

IMPCORE_BEGIN_NAMESPACE

//! Return a constant value.
/** This restraint is mostly for testing, but can also be used to make
    the total score look nicer.
 */
class IMPCOREEXPORT ConstantRestraint : public Restraint {
  Float v_;

 public:
  //! Add v to the total score.
  ConstantRestraint(Model *m, Float v);

  virtual double unprotected_evaluate(IMP::DerivativeAccumulator *accum)
      const IMP_OVERRIDE;
  virtual IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(ConstantRestraint);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_CONSTANT_RESTRAINT_H */
