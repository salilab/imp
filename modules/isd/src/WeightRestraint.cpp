/**
 *  \file isd/ WeightRestraint.cpp
 *  \brief
 *  Put description here
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <stdio.h>
#include <IMP/constants.h>
#include <IMP/isd/WeightRestraint.h>
#include <IMP/core/XYZ.h>
#include <IMP/isd/Weight.h>
#include <math.h>
#include <limits.h>
#include <IMP/base/random.h>
#include <iostream>

IMPISD_BEGIN_NAMESPACE

WeightRestraint::WeightRestraint
  (Particle *w, Float wmin, Float wmax, Float kappa):
      kernel::Restraint(w->get_model(), "WeightRestraint%1%"),
   w_(w), wmin_(wmin), wmax_(wmax), kappa_(kappa) {}

double WeightRestraint::
                 unprotected_evaluate(DerivativeAccumulator *accum) const
{
 // retrieve weights
 algebra::VectorKD weight = Weight(w_).get_weights();

 Float dw = 0.;

 for (unsigned i=0; i< weight.get_dimension(); ++i){
  if(weight[i] > wmax_)       dw += (weight[i] - wmax_) * (weight[i] - wmax_);
  else if (weight[i] < wmin_) dw += (wmin_ - weight[i]) * (wmin_ - weight[i]);
 }

 if (accum)
 {
 }

 return 0.5 * kappa_ * dw;
}

/* Return all particles whose attributes are read by the restraints. To
   do this, ask the pair score what particles it uses.*/
ModelObjectsTemp  WeightRestraint::do_get_inputs() const
{
  ParticlesTemp ret;
  ret.push_back(w_);
  return ret;
}

IMPISD_END_NAMESPACE
