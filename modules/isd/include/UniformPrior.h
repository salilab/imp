/**
 *  \file IMP/isd/UniformPrior.h
 *  \brief A restraint on a scale parameter.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_UNIFORM_PRIOR_H
#define IMPISD_UNIFORM_PRIOR_H

#include "isd_config.h"
#include <IMP/SingletonScore.h>
#include <IMP/kernel/Restraint.h>

IMPISD_BEGIN_NAMESPACE

//! Uniform distribution with harmonic boundaries

class IMPISDEXPORT UniformPrior : public kernel::Restraint
{
  base::Pointer<Particle> p_;
  Float upperb_;
  Float lowerb_;
  Float k_;

public:
  //! Create the restraint.
  UniformPrior(IMP::kernel::Model* m, Particle *p, Float k,
            Float upperb, Float lowerb, std::string name="UniformPrior%1%");

  /** This macro declares the basic needed methods: evaluate and show
   */
  virtual double
  unprotected_evaluate(IMP::kernel::DerivativeAccumulator *accum)
     const IMP_OVERRIDE;
  virtual IMP::kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(UniformPrior);

   /* call for probability */
  virtual double get_probability() const
  {
    return exp(-unprotected_evaluate(nullptr));
  }

};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_UNIFORM_PRIOR_H */
