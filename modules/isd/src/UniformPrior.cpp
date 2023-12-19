/**
 *  \file IMP/isd/UniformPrior.cpp
 *  \brief Restrain a scale particle with log(scale)
 *
 *  Copyright 2007-2023 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/Scale.h>
#include <IMP/isd/UniformPrior.h>
#include <cmath>

IMPISD_BEGIN_NAMESPACE

UniformPrior::UniformPrior(IMP::Model *m, Particle *p, double k,
                           double upperb, double lowerb, std::string name):
    Restraint(m, name), pi_(p->get_index()), upperb_(upperb),
    lowerb_(lowerb), k_(k) {}

/* Apply the score if it's a scale decorator.
 */
double
UniformPrior::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  IMP::isd::Scale sig(get_model(), pi_);
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
  return ModelObjectsTemp(1, get_model()->get_particle(pi_));
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
  ri->add_particle_indexes("particle", ParticleIndexes(1, pi_));
  return ri.release();
}

IMP_OBJECT_SERIALIZE_IMPL(IMP::isd::UniformPrior);

IMPISD_END_NAMESPACE
