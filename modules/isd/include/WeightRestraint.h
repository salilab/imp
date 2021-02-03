/**
 *  \file IMP/isd/WeightRestraint.h
 *  \brief Put description here
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_WEIGHT_RESTRAINT_H
#define IMPISD_WEIGHT_RESTRAINT_H
#include <IMP/isd/Scale.h>
#include "isd_config.h"
#include <IMP/Restraint.h>

IMPISD_BEGIN_NAMESPACE
/** A restraint for in-vivo ensemble FRET data
 */

class IMPISDEXPORT WeightRestraint : public Restraint {
  Particle *w_;
  Float wmin_;
  Float wmax_;
  Float kappa_;

 public:
  //! Create the restraint.
  WeightRestraint(Particle *w, Float wmin, Float wmax, Float kappa);

  virtual double unprotected_evaluate(IMP::DerivativeAccumulator *accum)
      const IMP_OVERRIDE;
  virtual IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(WeightRestraint);
};

IMPISD_END_NAMESPACE

#endif /* IMPISD_WEIGHT_RESTRAINT_H */
