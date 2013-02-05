/**
 *  \file IMP/kernel/Particle.h
 *  \brief Classes to handle individual model particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_PARTICLE_H
#define IMPKERNEL_PARTICLE_H

#include <IMP/kernel/kernel_config.h>
#include "declare_Particle.h"
#include "Model.h"


IMPKERNEL_BEGIN_NAMESPACE
#ifndef IMP_DOXYGEN

inline bool Particle::get_is_active() const {
  IMP_CHECK_OBJECT(this);
  return get_is_part_of_model();
}

inline ParticleIndex Particle::get_index() const {
  return id_;
}


IMP_PARTICLE_ATTRIBUTE_TYPE_DEF(Float, float, Float);
IMP_PARTICLE_ATTRIBUTE_TYPE_DEF(Int, int, Int);
IMP_PARTICLE_ATTRIBUTE_TYPE_DEF(String, string, String);
IMP_PARTICLE_ATTRIBUTE_TYPE_DEF(Object, object, base::Object*);
IMP_PARTICLE_ATTRIBUTE_TYPE_DEF(WeakObject, weak_object, base::Object*);


inline void Particle::add_attribute(FloatKey name,
                                    const Float initial_value, bool optimized){
  IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");
  get_model()->add_attribute(name, id_, initial_value);
  get_model()->set_is_optimized(name, id_, optimized);
}
inline void Particle::add_to_derivative(FloatKey key, Float value,
                                        const DerivativeAccumulator &da) {
  IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");
  get_model()->add_to_derivative(key, id_, value, da);
}
inline void Particle::set_is_optimized(FloatKey k, bool tf) {
  IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");
  return get_model()->set_is_optimized(k, id_, tf);
}
inline bool Particle::get_is_optimized(FloatKey k) const {
  IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");
  return get_model()->get_is_optimized(k, id_);
}
inline Float Particle::get_derivative(FloatKey name) const {
  IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");
  return get_model()->get_derivative(name, id_);
}
inline void Particle::add_attribute(ParticleIndexKey k, Particle *v) {
  IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");
  get_model()->add_attribute(k, id_, v->get_index());
}
inline bool Particle::has_attribute(ParticleIndexKey k) {
  IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");
  return get_model()->get_has_attribute(k, id_);
}
inline void Particle::set_value(ParticleIndexKey k, Particle *v) {
  IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");
  get_model()->set_attribute(k, id_, v->get_index());
}
inline Particle *Particle::get_value(ParticleIndexKey k) const {
  IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");
  return get_model()->get_particle(get_model()->get_attribute(k, id_));
}
inline void Particle::remove_attribute(ParticleIndexKey k) {
  IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");
  get_model()->remove_attribute(k, id_);
}
inline ParticleIndexKeys Particle::get_particle_keys() const {
  IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");
  return get_model()->internal::ParticleAttributeTable::get_attribute_keys(id_);
}
#endif

IMPKERNEL_END_NAMESPACE

#endif  /* IMPKERNEL_PARTICLE_H */
