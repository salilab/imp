/**
 *  \file IMP/spb/DiameterRgyrRestraint.h
 *  \brief Diameter Restraint
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPSPB_DIAMETER_RGYR_RESTRAINT_H
#define IMPSPB_DIAMETER_RGYR_RESTRAINT_H

#include "IMP/Restraint.h"
#include <IMP/spb/spb_config.h>
#include <IMP/Particle.h>
#include <IMP/base_types.h>
#include <map>
#include <string>

IMPSPB_BEGIN_NAMESPACE

//! Diameter and radius of gyration Restraint
/** Restraint to fix the diameter based on radius of gyration from SAXS

 */
class IMPSPBEXPORT DiameterRgyrRestraint : public Restraint {
 private:
  Particles ps_;
  Float diameter_;
  Float rgyr_;
  Float kappa_;

 public:
  DiameterRgyrRestraint(Particles ps, Float diameter, Float rgyr, Float kappa);

  virtual double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const
      IMP_OVERRIDE;
  IMP::ModelObjectsTemp do_get_inputs() const;

  IMP_OBJECT_METHODS(DiameterRgyrRestraint);
};

IMPSPB_END_NAMESPACE

#endif /* IMPSPB_DIAMETER_RGYR_RESTRAINT_H */
