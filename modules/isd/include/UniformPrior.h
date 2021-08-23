/**
 *  \file IMP/isd/UniformPrior.h
 *  \brief A restraint on a scale parameter.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_UNIFORM_PRIOR_H
#define IMPISD_UNIFORM_PRIOR_H

#include <IMP/isd/isd_config.h>
#include <IMP/SingletonScore.h>
#include <IMP/Restraint.h>

IMPISD_BEGIN_NAMESPACE

//! Uniform distribution with harmonic boundaries

class IMPISDEXPORT UniformPrior : public Restraint
{
  Pointer<Particle> p_;
  Float upperb_;
  Float lowerb_;
  Float k_;

public:
  //! Create the restraint.
  UniformPrior(IMP::Model* m, Particle *p, Float k,
            Float upperb, Float lowerb, std::string name="UniformPrior%1%");

  virtual double
  unprotected_evaluate(IMP::DerivativeAccumulator *accum)
     const IMP_OVERRIDE;
  virtual IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(UniformPrior);

   /* call for probability */
  virtual double get_probability() const
  {
    return exp(-unprotected_evaluate(nullptr));
  }

  //! \return Information for writing to RMF files
  RestraintInfo *get_static_info() const IMP_OVERRIDE;

  //! \return Information for writing to RMF files
  RestraintInfo *get_dynamic_info() const IMP_OVERRIDE;

};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_UNIFORM_PRIOR_H */
