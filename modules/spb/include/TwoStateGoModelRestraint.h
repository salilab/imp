/**
 *  \file IMP/spb/TwoStateGoModelRestraint.h
 *  \brief Two-state Go-Model Restraint
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 */

#ifndef IMPSPB_TWO_STATE_GO_MODEL_RESTRAINT_H
#define IMPSPB_TWO_STATE_GO_MODEL_RESTRAINT_H

#include "IMP/Restraint.h"
#include "spb_config.h"
#include <IMP/Particle.h>
#include <IMP/base_types.h>
#include <map>
#include <string>

IMPSPB_BEGIN_NAMESPACE

//! Two-state Go-Model Restraint
class IMPSPBEXPORT TwoStateGoModelRestraint : public Restraint {
 private:
  Particles ps_;
  Float Beta_;
  Float DeltaV_;
  std::map<IntPair, Float> native_attrA_;
  std::map<IntPair, Float> native_attrB_;

  void set_parameters(Particles psA, Particles psB, Float cutoff);
  double get_contribution(Float dist, Float dist0) const;
  double get_native_potential(const std::map<IntPair, Float>& native) const;

 public:
  TwoStateGoModelRestraint(Particles ps, Particles psA, Particles psB,
                           Float Beta, Float Delta, Float Cutoff);

  virtual double unprotected_evaluate(IMP::DerivativeAccumulator* accum) const
      IMP_OVERRIDE;
  virtual IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;

  // IMP_RESTRAINT(TwoStateGoModelRestraint);
  IMP_OBJECT_METHODS(TwoStateGoModelRestraint);
};

IMPSPB_END_NAMESPACE

#endif /* IMPSPB_TWO_STATE_GO_MODEL_RESTRAINT_H */
