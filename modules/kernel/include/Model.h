/**
 *  \file IMP/Model.h   \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_MODEL_H
#define IMPKERNEL_MODEL_H

#include "kernel_config.h"
#include "declare_Model.h"
#include "log.h"

IMP_BEGIN_NAMESPACE
#ifndef IMP_DOXYGEN
inline bool Model::get_has_dependencies() const {
  return !RestraintTracker::get_is_dirty()
    && !ScoringFunctionTracker::get_is_dirty()
    && !dependencies_dirty_;
}


inline unsigned int Model::get_number_of_particles() const {
  return get_particles().size();
}
inline Particle* Model::get_particle(ParticleIndex p) const  {
  IMP_USAGE_CHECK(particle_index_.size() > get_as_unsigned_int(p),
                  "Invalid particle requested");
  IMP_USAGE_CHECK(particle_index_[p],
                  "Invalid particle requested");
  return particle_index_[p];
}
#endif

IMP_END_NAMESPACE
#include "ScoringFunction.h"
#include "Restraint.h"

#endif  /* IMPKERNEL_MODEL_H */
