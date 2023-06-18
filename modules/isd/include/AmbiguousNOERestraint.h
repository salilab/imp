/**
 *  \file IMP/isd/AmbiguousNOERestraint.h
 *  \brief A lognormal restraint that uses the ISPA model to model NOE-derived
 *  distance fit.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_AMBIGUOUS_NOE_RESTRAINT_H
#define IMPISD_AMBIGUOUS_NOE_RESTRAINT_H

#include <IMP/isd/isd_config.h>
#include <IMP/SingletonScore.h>
#include <IMP/core/XYZ.h>
#include <IMP/Restraint.h>
#include <IMP/PairContainer.h>
#include <IMP/isd/Scale.h>

IMPISD_BEGIN_NAMESPACE

//! Ambiguous NOE distance restraint between a number of pairs of particles.
class IMPISDEXPORT AmbiguousNOERestraint : public Restraint {
  Pointer<PairContainer> pc_;
  ParticleIndex sigma_;
  ParticleIndex gamma_;
  double Vexp_;
  double chi_;
  void set_chi(double chi) { chi_ = chi; }

  friend class cereal::access;

  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Restraint>(this),
       pc_, sigma_, gamma_, Vexp_, chi_);
  }

  IMP_OBJECT_SERIALIZE_DECL(AmbiguousNOERestraint);

 public:
  //! Create the restraint.
  AmbiguousNOERestraint(Model *m, PairContainer *pc,
                        ParticleIndexAdaptor sigma, ParticleIndexAdaptor gamma,
                        double Iexp);

  AmbiguousNOERestraint() {}

  /* call for probability */
  double get_probability() const { return exp(-unprotected_evaluate(nullptr)); }

  double get_chi() const { return chi_; }

  virtual double unprotected_evaluate(IMP::DerivativeAccumulator *accum)
      const override;
  virtual IMP::ModelObjectsTemp do_get_inputs() const override;
  IMP_OBJECT_METHODS(AmbiguousNOERestraint);
};

IMPISD_END_NAMESPACE

#endif /* IMPISD_AMBIGUOUS_NOE_RESTRAINT_H */
