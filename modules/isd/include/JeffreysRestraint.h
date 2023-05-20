/**
 *  \file IMP/isd/JeffreysRestraint.h
 *  \brief A restraint on a scale parameter.
 *
 *  Copyright 2007-2023 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_JEFFREYS_RESTRAINT_H
#define IMPISD_JEFFREYS_RESTRAINT_H

#include <IMP/isd/isd_config.h>
#include <IMP/SingletonScore.h>
#include <IMP/Restraint.h>
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>

IMPISD_BEGIN_NAMESPACE

//! Score a Scale particle with log(scale)
//! the probability is 1/scale

class IMPISDEXPORT JeffreysRestraint : public Restraint {
  ParticleIndex pi_;

  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Restraint>(this), pi_);
  }
  IMP_OBJECT_SERIALIZE_DECL(JeffreysRestraint);

 public:
  //! Create the restraint.
  JeffreysRestraint(Model *m, Particle *p);

  JeffreysRestraint() {}

  virtual double unprotected_evaluate(IMP::DerivativeAccumulator *accum)
      const override;
  virtual IMP::ModelObjectsTemp do_get_inputs() const override;

  IMP_OBJECT_METHODS(JeffreysRestraint);

  /* call for probability */
  virtual double get_probability() const {
    return exp(-unprotected_evaluate(nullptr));
  }
};

IMPISD_END_NAMESPACE

#endif /* IMPISD_JEFFREYS_RESTRAINT_H */
