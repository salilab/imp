/**
 *  \file isd/GaussianRestraint.h
 *  \brief A lognormal restraint that uses the ISPA model to model NOE-derived
 *  distance fit.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_GAUSSIAN_RESTRAINT_H
#define IMPISD_GAUSSIAN_RESTRAINT_H

#include "isd_config.h"
#include <IMP/Particle.h>
#include <IMP/isd/ISDRestraint.h>

IMPISD_BEGIN_NAMESPACE

//! Normal probability distribution as a restraint
/**
    The source code is as follows:
    \include GaussianRestraint.h
    \include GaussianRestraint.cpp
*/
class IMPISDEXPORT GaussianRestraint : public ISDRestraint
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
  GaussianRestraint(Particle *x, Particle *mu, Particle *sigma);
  GaussianRestraint(double x, Particle *mu, Particle *sigma);
  GaussianRestraint(Particle *x, double mu, Particle *sigma);
  GaussianRestraint(Particle *x, Particle *mu, double sigma);
  GaussianRestraint(double x, double mu, Particle *sigma);
  GaussianRestraint(Particle *x, double mu, double sigma);
  GaussianRestraint(double x, Particle *mu, double sigma);

  /* call for probability */
  double get_probability() const
  {
    return exp(-unprotected_evaluate(NULL));
  }

  double get_chi() const
  {return chi_; }


  IMP_RESTRAINT(GaussianRestraint);

};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_GAUSSIAN_RESTRAINT_H */
