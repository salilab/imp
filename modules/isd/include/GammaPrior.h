/**
 *  \file IMP/isd/GammaPrior.h
 *  \brief A restraint on a scale parameter.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_GAMMA_PRIOR_H
#define IMPISD_GAMMA_PRIOR_H

#include "isd_config.h"
#include <IMP/SingletonScore.h>
#include <IMP/Restraint.h>

IMPISD_BEGIN_NAMESPACE

//! Uniform distribution with harmonic boundaries

class IMPISDEXPORT GammaPrior : public Restraint
{
  Pointer<Particle> p_;
  Float theta_;
  Float k_;

public:
  //! Create the restraint.
  GammaPrior(IMP::Model* m, Particle *p, Float k,
            Float theta, std::string name="GammaPrior%1%");

  virtual double
  unprotected_evaluate(IMP::DerivativeAccumulator *accum)
     const IMP_OVERRIDE;
  virtual double evaluate_at(Float val) const;
  virtual IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(GammaPrior);

   /* call for probability */
  virtual double get_probability() const
  {
    return exp(-unprotected_evaluate(nullptr));
  }

};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_GAMMA_PRIOR_H */
