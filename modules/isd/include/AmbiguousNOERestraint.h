/**
 *  \file IMP/isd/AmbiguousNOERestraint.h
 *  \brief A lognormal restraint that uses the ISPA model to model NOE-derived
 *  distance fit.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_AMBIGUOUS_NOE_RESTRAINT_H
#define IMPISD_AMBIGUOUS_NOE_RESTRAINT_H

#include <IMP/isd/isd_config.h>
#include <IMP/SingletonScore.h>
#include <IMP/core/XYZ.h>
#include <IMP/kernel/Restraint.h>
#include <IMP/PairContainer.h>
#include <IMP/isd/Scale.h>

IMPISD_BEGIN_NAMESPACE

//! Ambiguous NOE distance restraint between a number of pairs of particles.
class IMPISDEXPORT AmbiguousNOERestraint : public kernel::Restraint {
  base::Pointer<PairContainer> pc_;
  base::Pointer<kernel::Particle> sigma_;
  base::Pointer<kernel::Particle> gamma_;
  double Vexp_;
  double chi_;
  void set_chi(double chi) { chi_ = chi; }

 public:
  //! Create the restraint.
  /** kernel::Restraints should store the particles they are to act on,
      preferably in a Singleton or PairContainer as appropriate.
   */
  AmbiguousNOERestraint(kernel::Model *m, PairContainer *pc,
                        kernel::Particle *sigma, kernel::Particle *gamma,
                        double Iexp);

  /* call for probability */
  double get_probability() const { return exp(-unprotected_evaluate(nullptr)); }

  double get_chi() const { return chi_; }

  /** This macro declares the basic needed methods: evaluate and show
   */
  virtual double unprotected_evaluate(IMP::kernel::DerivativeAccumulator *accum)
      const IMP_OVERRIDE;
  virtual IMP::kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(AmbiguousNOERestraint);
};

IMPISD_END_NAMESPACE

#endif /* IMPISD_AMBIGUOUS_NOE_RESTRAINT_H */
