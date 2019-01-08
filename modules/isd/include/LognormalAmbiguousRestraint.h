/**
 *  \file IMP/isd/LognormalAmbiguousRestraint.h
 *  \brief A sigmoid shaped restraint between
 *  residues with discrete classifier
 *  and ambiguous assignment. To be used with
 *  cross-linking mass-spectrometry data.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_LOGNORMAL_AMBIGUOUS_RESTRAINT_H
#define IMPISD_LOGNORMAL_AMBIGUOUS_RESTRAINT_H

#include <IMP/isd/Scale.h>
#include "isd_config.h"
#include <IMP/Restraint.h>
#include <IMP/PairContainer.h>

IMPISD_BEGIN_NAMESPACE

/** A restraint for ambiguous cross-linking MS data.
    \f$ z_i \f$ is the discrete classifier,
    \f$ \sigma_G \f$ is the uncertainty estimate for good data points,
    \f$ \sigma_B \f$ is the uncertainty estimate for bad data points,
    \f$ k_i \f$ is the index of the restraint to be
    picked up from the ambiguous set, and
    \f$ \lambda_0 \f$ is the maximum length for the cross-linker.

    Usage: Construct the ambiguous set using
    LognormalAmbiguousRestraint(\f$ z_i \f$, \f$ \sigma_G \f$,
    \f$ \sigma_B \f$, \f$ k_i \f$, \f$ \lambda_0 \f$).
    Pass individual cross-linked particles to
    add_contribution(part0, part1).
    The marginal likelihood is modeled by a sigmoid function:

    \f[p(D|X,I) = \frac{1}{\lambda_0+\sigma(\sqrt(\pi/2)-1)}
    [1+\theta(r_k-\lambda_0+\sigma)[\exp(-\frac{(r_k-
    \lambda_0+\sigma)^2}{2\sigma^2})]]
    \f]

    where \f$ \sigma=z_i\sigma_G+(1-z_i)\sigma_B \f$
    and \f$ r_k \f$ is the distance between part0 and part1.
 */
class IMPISDEXPORT LognormalAmbiguousRestraint : public Restraint
{
  Pointer<Particle> p1_;
  Pointer<Particle> p2_;
  Pointer<Particle> ki_;
  Pointer<Particle> sigmaG_;
  Particles omegas_;
  Floats lexp_;

public:
  //! Create the restraint.
  /** Restraints should store the particles they are to act on,
      preferably in a Singleton or PairContainer as appropriate.
   */
  LognormalAmbiguousRestraint
    (Particle *p1, Particle *p2, Particle *ki, Particle *sigmaG);


  // add a contribution: simple case
  void add_contribution(double lexp, Particle *omega);

  unsigned get_number_of_contributions() const {return lexp_.size();}

  void draw_k_from_posterior(double kt);

  /* call for probability */
  double get_probability() const
  {
    return exp(-unprotected_evaluate(NULL));
  }

  virtual double unprotected_evaluate(IMP::DerivativeAccumulator *accum)
      const IMP_OVERRIDE;
  virtual IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(LognormalAmbiguousRestraint);
};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_LOGNORMAL_AMBIGUOUS_RESTRAINT_H */
