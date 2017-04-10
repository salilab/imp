/**
 *  \file IMP/isd/AmbiguousNOERestraint.h
 *  \brief A lognormal restraint that uses the ISPA model to model NOE-derived
 *  distance fit.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
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

 public:
  //! Create the restraint.
  AmbiguousNOERestraint(Model *m, PairContainer *pc,
                        ParticleIndexAdaptor sigma, ParticleIndexAdaptor gamma,
                        double Iexp);

  /* call for probability */
  double get_probability() const { return exp(-unprotected_evaluate(nullptr)); }

  double get_chi() const { return chi_; }

  virtual double unprotected_evaluate(IMP::DerivativeAccumulator *accum)
      const IMP_OVERRIDE;
  virtual IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(AmbiguousNOERestraint);
};

IMPISD_END_NAMESPACE

#endif /* IMPISD_AMBIGUOUS_NOE_RESTRAINT_H */
