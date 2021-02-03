/**
 *  \file IMP/isd/NOERestraint.h
 *  \brief A lognormal restraint that uses the ISPA model to model NOE-derived
 *  distance fit.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_NOE_RESTRAINT_H
#define IMPISD_NOE_RESTRAINT_H

#include <IMP/isd/isd_config.h>
#include <IMP/SingletonScore.h>
#include <IMP/core/XYZ.h>
#include <IMP/Restraint.h>
#include <IMP/PairContainer.h>
#include <IMP/PairScore.h>

IMPISD_BEGIN_NAMESPACE

//! Apply an NOE distance restraint between two particles.
class IMPISDEXPORT NOERestraint : public Restraint {
  Pointer<Particle> p0_;
  Pointer<Particle> p1_;
  Pointer<Particle> sigma_;
  Pointer<Particle> gamma_;
  double Vexp_;
  double chi_;
  void set_chi(double chi) { chi_ = chi; }

 public:
  //! Create the restraint.
  /** Restraints should store the particles they are to act on,
      preferably in a Singleton or PairContainer as appropriate.
   */
  NOERestraint(Model *m, Particle *p0, Particle *p1, Particle *sigma,
               Particle *gamma, double Iexp);

  /* call for probability */
  double get_probability() const { return exp(-unprotected_evaluate(nullptr)); }

  double get_chi() const { return chi_; }

  virtual double unprotected_evaluate(IMP::DerivativeAccumulator *accum)
      const IMP_OVERRIDE;
  virtual IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(NOERestraint);
};

IMPISD_END_NAMESPACE

#endif /* IMPISD_NOE_RESTRAINT_H */
