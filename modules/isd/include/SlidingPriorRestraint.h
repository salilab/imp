/**
 *  \file IMP/isd/SlidingPriorRestraint.h
 *  \brief A restraint on a scale parameter.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_SLIDING_PRIOR_RESTRAINT_H
#define IMPISD_SLIDING_PRIOR_RESTRAINT_H

#include <IMP/isd/isd_config.h>
#include <IMP/SingletonScore.h>
#include <IMP/isd/ISDRestraint.h>
#include <IMP/restraint_macros.h>

IMPISD_BEGIN_NAMESPACE

//! Score a Scale particle with unnormalized probability.
/** \f[\frac{1}{\sigma_{q}} \exp\left(+\frac{(q-qmin)^2}{2 \sigma_q^2}\right)
    \f]
    where q is between qmin and qmax.
 */
class IMPISDEXPORT SlidingPriorRestraint : public ISDRestraint
{
  Pointer<Particle> p_;
  double qmin_, qmax_, sq_;

public:
  //! Create the restraint.
  SlidingPriorRestraint(Particle *p, double qmin, double qmax, double sq);

  /** This macro declares the basic needed methods: evaluate and show
   */
  IMP_RESTRAINT(SlidingPriorRestraint);

  //! Call for probability
  virtual double get_probability() const
  {
    return exp(-unprotected_evaluate(nullptr));
  }

};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_SLIDING_PRIOR_RESTRAINT_H */
