/**
 *  \file IMP/isd/LognormalRestraint.h
 *  \brief A lognormal restraint that uses the ISPA model to model NOE-derived
 *  distance fit.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_LOGNORMAL_RESTRAINT_H
#define IMPISD_LOGNORMAL_RESTRAINT_H

#include <IMP/restraint_macros.h>
#include <IMP/isd/isd_config.h>
#include <IMP/Particle.h>
#include <IMP/isd/ISDRestraint.h>

IMPISD_BEGIN_NAMESPACE

//! Normal probability distribution as a restraint
class IMPISDEXPORT LognormalRestraint : public ISDRestraint
{
private:
  Pointer<Particle> px_;
  double x_;
  Pointer<Particle> pmu_;
  double mu_;
  Pointer<Particle> psigma_;
  double sigma_;
  bool isx_, ismu_, issigma_; // true if it's a particle
  double chi_;

private:
  void set_chi(double chi) { chi_ = chi; }
  void check_particles();

public:
  //! Gaussian restraint
  /** The restraint is a 3-particle restraint, linking together the mean,
   * observed value and variance, which can all be either Nuisances or doubles.
   * If F is the identity function, this is a gaussian (e.g. harmonic)
   * restraint.
   */
  LognormalRestraint(Particle *x, Particle *mu, Particle *sigma);
  LognormalRestraint(double x, Particle *mu, Particle *sigma);
  LognormalRestraint(Particle *x, double mu, Particle *sigma);
  LognormalRestraint(Particle *x, Particle *mu, double sigma);
  LognormalRestraint(double x, double mu, Particle *sigma);
  LognormalRestraint(Particle *x, double mu, double sigma);
  LognormalRestraint(double x, Particle *mu, double sigma);

  /* call for probability */
  double get_probability() const
  {
    return exp(-unprotected_evaluate(nullptr));
  }

  double get_chi() const
  {return chi_; }


  IMP_RESTRAINT(LognormalRestraint);

};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_LOGNORMAL_RESTRAINT_H */
