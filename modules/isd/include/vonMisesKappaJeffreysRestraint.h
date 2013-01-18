/**
 *  \file IMP/isd/vonMisesKappaJeffreysRestraint.h
 *  \brief Jeffreys prior for \f$\kappa\f$ in the von Mises distribution.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_VON_MISES_KAPPA_JEFFREYS_RESTRAINT_H
#define IMPISD_VON_MISES_KAPPA_JEFFREYS_RESTRAINT_H

#include <IMP/isd/isd_config.h>
#include <IMP/SingletonScore.h>
#include "ISDRestraint.h"
#include <IMP/restraint_macros.h>

IMPISD_BEGIN_NAMESPACE

//! Jeffreys prior for the \f$\kappa\f$ concentration parameter of a von Mises
//! distribution.
//! \f[p(\kappa) = \sqrt{\frac{I_1(\kappa)}{I_0(\kappa)}
//  \left[
//    \kappa
//    -\frac{I_1(\kappa)}{I_0(\kappa)}
//    -kappa\left(\frac{I_1(\kappa)}{I_0(\kappa)}\right)^2
//  \right]}\f]
//!

class IMPISDEXPORT vonMisesKappaJeffreysRestraint : public ISDRestraint
{
  Pointer<Particle> kappa_;
  double old_kappaval;
  double I0_,I1_;

public:
  //! Create the restraint.
  vonMisesKappaJeffreysRestraint(Particle *kappa);

  /** This macro declares the basic needed methods: evaluate and show
   */
  IMP_RESTRAINT(vonMisesKappaJeffreysRestraint);

virtual double get_probability() const;

private:

void update_bessel(double kappaval); // update memoized bessel values

};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_VON_MISES_KAPPA_JEFFREYS_RESTRAINT_H */
