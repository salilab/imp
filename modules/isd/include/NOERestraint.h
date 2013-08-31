/**
 *  \file IMP/isd/NOERestraint.h
 *  \brief A lognormal restraint that uses the ISPA model to model NOE-derived
 *  distance fit.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_NOE_RESTRAINT_H
#define IMPISD_NOE_RESTRAINT_H

#include <IMP/isd/isd_config.h>
#include <IMP/SingletonScore.h>
#include <IMP/core/XYZ.h>
#include <IMP/isd/ISDRestraint.h>
#include <IMP/PairContainer.h>
#include <IMP/PairScore.h>
#include <IMP/restraint_macros.h>

IMPISD_BEGIN_NAMESPACE

//! Apply an NOE distance restraint between two particles.
class IMPISDEXPORT NOERestraint : public ISDRestraint
{
  base::Pointer<kernel::Particle> p0_;
  base::Pointer<kernel::Particle> p1_;
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
  NOERestraint(kernel::Particle *p0, kernel::Particle *p1, kernel::Particle *sigma,
               kernel::Particle *gamma,double Iexp);

  /* call for probability */
  double get_probability() const
  {
    return exp(-unprotected_evaluate(nullptr));
  }

  double get_chi() const
  {return chi_; }


  /** This macro declares the basic needed methods: evaluate and show
   */
  virtual double
  unprotected_evaluate(IMP::kernel::DerivativeAccumulator *accum)
     const IMP_OVERRIDE;
  virtual IMP::kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(NOERestraint);

};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_NOE_RESTRAINT_H */
