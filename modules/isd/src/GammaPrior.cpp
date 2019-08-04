/**
 *  \file IMP/isd/GammaPrior.cpp
 *  \brief Restrain a scale particle with a gamma distribution
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/Scale.h>
#include <IMP/isd/GammaPrior.h>
#include <math.h>

IMPISD_BEGIN_NAMESPACE

GammaPrior::GammaPrior(IMP::Model *m, Particle *p, Float k, 
                         Float theta, std::string name):
    Restraint(m, name), p_(p), theta_(theta),  k_(k) {}


/* Apply the score if it's a scale decorator.
 */
double
GammaPrior::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  IMP::isd::Scale sig(p_);
  double prob=0.0;
  double s=sig.get_scale()/10;
  double gam = tgamma (k_)*std::pow(theta_,k_);
  prob = std::pow(s, k_-1) * std::exp(-s/theta_)/gam;
    
  if (accum) {
  }

  double score = -1*log (prob);
  return score;
}

double
GammaPrior::evaluate_at(Float val) const
{
  double prob=0.0;
  double gam = tgamma (k_)*std::pow(theta_,k_);
  prob = std::pow(val, k_-1) * std::exp(-val/theta_)/gam;
    
  double score = -1*log (prob);
  return score;
}

/* Return all particles whose attributes are read by the restraints. To
   do this, ask the pair score what particles it uses.*/
ModelObjectsTemp GammaPrior::do_get_inputs() const
{
  return ParticlesTemp(1,p_);
}

IMPISD_END_NAMESPACE
