/**
 *  \file TwoStateGoModelRestraint.h
 *  \brief Two-state Go-Model Restraint
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPMEMBRANE_TWO_STATE_GO_MODEL_RESTRAINT_H
#define IMPMEMBRANE_TWO_STATE_GO_MODEL_RESTRAINT_H

#include "membrane_config.h"
#include "IMP/Restraint.h"
#include "IMP/restraint_macros.h"
#include <IMP/Particle.h>
#include <IMP/base_types.h>
#include <string>
#include <map>

IMPMEMBRANE_BEGIN_NAMESPACE

//! Two-state Go-Model Restraint
/** Two-state Go-Model description here

 */
class IMPMEMBRANEEXPORT TwoStateGoModelRestraint : public Restraint
{

private:
Particles ps_;
Float Beta_;
Float DeltaV_;
std::map<IntPair,Float> native_attrA_;
std::map<IntPair,Float> native_attrB_;

void set_parameters (Particles psA, Particles psB, Float cutoff);
double get_contribution (Float dist, Float dist0) const;
double get_native_potential
 (const std::map< IntPair, Float>& native) const;

public:

  TwoStateGoModelRestraint(Particles ps, Particles psA, Particles psB,
   Float Beta, Float Delta, Float Cutoff);


  IMP_RESTRAINT(TwoStateGoModelRestraint);

};

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_TWO_STATE_GO_MODEL_RESTRAINT_H */
