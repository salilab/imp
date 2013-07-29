/**
 *  \file IMP/isd/JeffreysRestraint.h
 *  \brief A restraint on a scale parameter.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_JEFFREYS_RESTRAINT_H
#define IMPISD_JEFFREYS_RESTRAINT_H

#include <IMP/restraint_macros.h>
#include <IMP/isd/isd_config.h>
#include <IMP/SingletonScore.h>
#include <IMP/isd/ISDRestraint.h>

IMPISD_BEGIN_NAMESPACE

//! Score a Scale particle with log(scale)
//! the probability is 1/scale

class IMPISDEXPORT JeffreysRestraint : public ISDRestraint
{
  base::Pointer<Particle> p_;

public:
  //! Create the restraint.
  JeffreysRestraint(Particle *p);
  JeffreysRestraint(Model *m, Particle *p);

  /** This macro declares the basic needed methods: evaluate and show
   */
  virtual double
  unprotected_evaluate(IMP::kernel::DerivativeAccumulator *accum)
     const IMP_OVERRIDE;
  virtual IMP::kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(JeffreysRestraint);

   /* call for probability */
  virtual double get_probability() const
  {
    return exp(-unprotected_evaluate(nullptr));
  }

};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_JEFFREYS_RESTRAINT_H */
