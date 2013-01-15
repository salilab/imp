/**
 *  \file IMP/isd/AmbiguousNOERestraint.h
 *  \brief A lognormal restraint that uses the ISPA model to model NOE-derived
 *  distance fit.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_AMBIGUOUS_NOE_RESTRAINT_H
#define IMPISD_AMBIGUOUS_NOE_RESTRAINT_H

#include <IMP/isd/isd_config.h>
#include <IMP/SingletonScore.h>
#include <IMP/core/XYZ.h>
#include <IMP/restraint_macros.h>
#include <IMP/isd/ISDRestraint.h>
#include <IMP/PairContainer.h>
#include <IMP/isd/Scale.h>

IMPISD_BEGIN_NAMESPACE

//! Ambiguous NOE distance restraint between a number of pairs of particles.
class IMPISDEXPORT AmbiguousNOERestraint : public ISDRestraint
{
  Pointer<PairContainer> pc_;
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
  AmbiguousNOERestraint(PairContainer *pc, Particle *sigma, Particle *gamma,
                        double Iexp);

  /* call for probability */
  double get_probability() const
  {
    return exp(-unprotected_evaluate(nullptr));
  }

  double get_chi() const
  {return chi_; }


  /** This macro declares the basic needed methods: evaluate and show
   */
  IMP_RESTRAINT(AmbiguousNOERestraint);

};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_AMBIGUOUS_NOE_RESTRAINT_H */
