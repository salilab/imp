/**
 *  \file FixedRefiner.cpp
 *  \brief A particle refiner which returns a fixed set of particles.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/core/FixedRefiner.h"
#include <IMP/log.h>

IMPCORE_BEGIN_NAMESPACE

FixedRefiner::FixedRefiner(const ParticlesTemp &ps)
  : Refiner("FixedRefiner%d", true) {
  IMP_USAGE_CHECK(ps.size()>0, "cannot refine with empty particle list");
  IMP_LOG_VERBOSE("Created fixed particle refiner with "
                  << ps.size() << " particles" << std::endl);
  m_ = ps[0]->get_model();
  for (const auto &p : ps) {
    IMP_USAGE_CHECK(m_ == p->get_model(),
                    "refiner assumes all particles are from the same model");
    pis_.push_back(p->get_index());
  }
}

FixedRefiner::FixedRefiner(Model* m, const ParticleIndexes &pis)
  : Refiner("FixedRefiner%d", true), m_(m), pis_(pis) {
  IMP_LOG_VERBOSE("Created fixed particle refiner with "
                  << pis_.size() << " particles" << std::endl);
}

const ParticlesTemp
FixedRefiner::get_refined(Particle *) const
{
  ParticlesTemp ps;
  for (const auto &pi : pis_) {
    ps.push_back(m_->get_particle(pi));
  }
  return ps;
}


ModelObjectsTemp FixedRefiner::do_get_inputs(
    Model *, const ParticleIndexes &) const {
  return ModelObjectsTemp();
}

void FixedRefiner::set_model_from_id(uint32_t model_id)
{
  Model *m = Model::get_by_unique_id(model_id);
  if (!m) {
    IMP_THROW("Cannot unserialize FixedRefiner as it refers to a "
              "Model that does not exist", ValueException);
  } else {
    m_ = m;
  }

}

uint32_t FixedRefiner::get_model_id() const
{
  return m_->get_unique_id();
}

IMP_OBJECT_SERIALIZE_IMPL(IMP::core::FixedRefiner);

IMPCORE_END_NAMESPACE
