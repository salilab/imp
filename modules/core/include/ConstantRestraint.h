/**
 *  \file IMP/core/ConstantRestraint.h    \brief Constant restraint.
 *
 *  Just return a constant.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_CONSTANT_RESTRAINT_H
#define IMPCORE_CONSTANT_RESTRAINT_H

#include <IMP/core/core_config.h>

#include <IMP/Restraint.h>
#include <IMP/PairScore.h>
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>
#include <cereal/types/polymorphic.hpp>

IMPCORE_BEGIN_NAMESPACE

//! Return a constant value.
/** This restraint is mostly for testing, but can also be used to make
    the total score look nicer.
 */
class IMPCOREEXPORT ConstantRestraint : public Restraint {
  Float v_;

  friend class cereal::access;

  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Restraint>(this), v_);
  }
  IMP_OBJECT_SERIALIZE_DECL(ConstantRestraint);

 public:
  //! Add v to the total score.
  ConstantRestraint(Model *m, Float v);
  ConstantRestraint() {}

  virtual double unprotected_evaluate(IMP::DerivativeAccumulator *accum)
      const override;
  virtual IMP::ModelObjectsTemp do_get_inputs() const override;
  IMP_OBJECT_METHODS(ConstantRestraint);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_CONSTANT_RESTRAINT_H */
