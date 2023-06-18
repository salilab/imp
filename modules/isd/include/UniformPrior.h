/**
 *  \file IMP/isd/UniformPrior.h
 *  \brief A restraint on a scale parameter.
 *
 *  Copyright 2007-2023 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_UNIFORM_PRIOR_H
#define IMPISD_UNIFORM_PRIOR_H

#include <IMP/isd/isd_config.h>
#include <IMP/SingletonScore.h>
#include <IMP/Restraint.h>
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>

IMPISD_BEGIN_NAMESPACE

//! Uniform distribution with harmonic boundaries

class IMPISDEXPORT UniformPrior : public Restraint
{
  ParticleIndex pi_;
  double upperb_;
  double lowerb_;
  double k_;

  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Restraint>(this), pi_, upperb_, lowerb_, k_);
  }
  IMP_OBJECT_SERIALIZE_DECL(UniformPrior);

public:
  //! Create the restraint.
  UniformPrior(IMP::Model* m, Particle *p, double k, double upperb,
               double lowerb, std::string name="UniformPrior%1%");

  UniformPrior() {}

  virtual double
  unprotected_evaluate(IMP::DerivativeAccumulator *accum)
     const override;
  virtual IMP::ModelObjectsTemp do_get_inputs() const override;
  IMP_OBJECT_METHODS(UniformPrior);

   /* call for probability */
  virtual double get_probability() const
  {
    return exp(-unprotected_evaluate(nullptr));
  }

  //! \return Information for writing to RMF files
  RestraintInfo *get_static_info() const override;

  //! \return Information for writing to RMF files
  RestraintInfo *get_dynamic_info() const override;

};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_UNIFORM_PRIOR_H */
