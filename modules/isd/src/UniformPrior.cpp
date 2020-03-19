/**
 *  \file IMP/isd/UniformPrior.cpp
 *  \brief Restrain a scale particle with log(scale)
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/Scale.h>
#include <IMP/isd/UniformPrior.h>
#include <math.h>

IMPISD_BEGIN_NAMESPACE

UniformPrior::UniformPrior(IMP::Model *m, Particle *p, Float k, 
                         Float upperb, Float lowerb, std::string name):
    Restraint(m, name), p_(p), upperb_(upperb), lowerb_(lowerb), k_(k) {}


/* Apply the score if it's a scale decorator.
 */
double
UniformPrior::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  IMP::isd::Scale sig(p_);
  double score=0.0;
  double s=sig.get_scale();

  
  if (s <= lowerb_) {
     score=0.5*k_*(s-lowerb_)*(s-lowerb_);
  }
  else if (s >= upperb_){
     score=0.5*k_*(s-upperb_)*(s-upperb_);  
  }
  
  if (accum) {
  }
  return score;
}

/* Return all particles whose attributes are read by the restraints. To
   do this, ask the pair score what particles it uses.*/
ModelObjectsTemp UniformPrior::do_get_inputs() const
{
  return ParticlesTemp(1,p_);
}

RestraintInfo *UniformPrior::get_static_info() const {
  IMP_NEW(RestraintInfo, ri, ());
  ri->add_string("type", "IMP.isd.UniformPrior");
  ri->add_float("force constant", k_);
  ri->add_float("lower bound", lowerb_);
  ri->add_float("upper bound", upperb_);
  return ri.release();
}

RestraintInfo *UniformPrior::get_dynamic_info() const {
  IMP_NEW(RestraintInfo, ri, ());
  ri->add_particle_indexes("particle", ParticleIndexes(1, p_->get_index()));
  return ri.release();
}

IMPISD_END_NAMESPACE
