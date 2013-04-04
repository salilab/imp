/**
 *  \file rigid_bodies.h
 *  \brief utilities for rigid bodies.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_INTERNAL_RIGID_BODIES_H
#define IMPCORE_INTERNAL_RIGID_BODIES_H

#include <IMP/core/core_config.h>
#include "../XYZ.h"
#include "../Hierarchy.h"
#include <IMP/algebra/Sphere3D.h>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

struct RigidBodyData {
  FloatKeys child_keys_;
  FloatKeys quaternion_;
  FloatKeys torque_;
  FloatKeys lquaternion_;
  ParticleIndexesKey members_;
  ParticleIndexesKey body_members_;
  ParticleIndexKey body_;
  ObjectKey refkey_;
  RigidBodyData() {
    child_keys_.resize(3);
    std::string pre="rigid_body_";
    // rigid body internal coordinates are currently special cased
    child_keys_[0]= FloatKey(4);
    child_keys_[1]= FloatKey(5);
    child_keys_[2]= FloatKey(6);
    quaternion_.resize(4);
    quaternion_[0]= FloatKey((pre+"quaternion_0").c_str());
    quaternion_[1]= FloatKey((pre+"quaternion_1").c_str());
    quaternion_[2]= FloatKey((pre+"quaternion_2").c_str());
    quaternion_[3]= FloatKey((pre+"quaternion_3").c_str());
    torque_.resize(3);
    torque_[0]= FloatKey((pre+"torque_0").c_str());
    torque_[1]= FloatKey((pre+"torque_1").c_str());
    torque_[2]= FloatKey((pre+"torque_2").c_str());
    lquaternion_.resize(4);
    lquaternion_[0]= FloatKey((pre+"local_quaternion_0").c_str());
    lquaternion_[1]= FloatKey((pre+"local_quaternion_1").c_str());
    lquaternion_[2]= FloatKey((pre+"local_quaternion_2").c_str());
    lquaternion_[3]= FloatKey((pre+"local_quaternion_3").c_str());
    refkey_= ObjectKey("rigid body representation");
    members_= ParticleIndexesKey("rigid body members");
    body_members_= ParticleIndexesKey("rigid body body members");
    body_= ParticleIndexKey("rigid body");
  }
};

IMPCOREEXPORT const RigidBodyData &rigid_body_data();


inline void set_model_ranges(Model *m) {
  m->set_range(rigid_body_data().quaternion_[0], FloatRange(0,1));
  m->set_range(rigid_body_data().quaternion_[1], FloatRange(0,1));
  m->set_range(rigid_body_data().quaternion_[2], FloatRange(0,1));
  m->set_range(rigid_body_data().quaternion_[3], FloatRange(0,1));
}

inline bool get_has_required_attributes_for_body(Model *m,
                                                 ParticleIndex pi) {
  IMP_USAGE_CHECK((m->get_has_attribute(rigid_body_data().quaternion_[0], pi)
                   && m->get_has_attribute(rigid_body_data().quaternion_[1], pi)
                   && m->get_has_attribute(rigid_body_data().quaternion_[2], pi)
                   && m->get_has_attribute(rigid_body_data().quaternion_[3], pi)
                   && XYZ::particle_is_instance(m->get_particle(pi)))
                  || (!m->get_has_attribute(rigid_body_data().quaternion_[0],
                                            pi)
                      && !m->get_has_attribute(rigid_body_data().quaternion_[1],
                                               pi)
                      && !m->get_has_attribute(rigid_body_data().quaternion_[2],
                                               pi)
                      && !m->get_has_attribute(rigid_body_data().quaternion_[3],
                                               pi)),
            "Particle should have all of quaterion attributes or none");
  return m->get_has_attribute(rigid_body_data().quaternion_[0], pi);
}



inline bool
get_has_required_attributes_for_member(Model *m, ParticleIndex p) {
  // make cheaper
  if (!m->get_has_attribute(rigid_body_data().body_, p)) return false;
  for (unsigned int i=0; i< 3; ++i) {
    if (!m->get_has_attribute(rigid_body_data().child_keys_[i], p)){
      return false;
    }
  }
  for (unsigned int i=0; i< 3; ++i) {
    if (!m->get_has_attribute(XYZ::get_coordinate_key(i), p))
      return false;
  }
  return true;
}

inline bool
get_has_required_attributes_for_body_member(Particle *p) {
  if (!p->has_attribute(rigid_body_data().body_)) return false;
  for (unsigned int i=0; i< 4; ++i) {
    if (!p->has_attribute(rigid_body_data().lquaternion_[i])) return false;
  }
  return get_has_required_attributes_for_member(p->get_model(), p->get_index());
}

inline void add_required_attributes_for_body(Particle *p) {
  for (unsigned int i=0; i< 4; ++i) {
    p->add_attribute(rigid_body_data().quaternion_[i], 0);
    p->get_model()->set_range(rigid_body_data().quaternion_[i],
                              FloatRange(0,1));
  }
  for (unsigned int i=0; i< 3; ++i) {
    p->add_attribute(rigid_body_data().torque_[i], 0);
  }
  if (!XYZ::particle_is_instance(p)) {
      XYZ::setup_particle(p);
  }
}
inline void remove_required_attributes_for_body(Particle *p) {
  for (unsigned int i=0; i< 4; ++i) {
    p->remove_attribute(rigid_body_data().quaternion_[i]);
  }
  for (unsigned int i=0; i< 3; ++i) {
    p->remove_attribute(rigid_body_data().torque_[i]);
  }
  if (p->get_model()->get_has_attribute(internal::rigid_body_data().members_,
                                        p->get_index())) {
    p->get_model()->remove_attribute(internal::rigid_body_data().members_,
                                     p->get_index());
  }
  if (p->get_model()
      ->get_has_attribute(internal::rigid_body_data().body_members_,
                                        p->get_index())) {
    p->get_model()
      ->remove_attribute(internal::rigid_body_data().body_members_,
                         p->get_index());
  }
}

inline void add_required_attributes_for_member(Particle *p,
                                               Particle *rb) {
  for (unsigned int i=0; i< 3; ++i) {
    p->add_attribute(rigid_body_data().child_keys_[i], 0);
  }
  IMP_INTERNAL_CHECK(p->get_model()->get_internal_coordinates(p->get_index())
                     .get_magnitude() < .01, "Bad initialization");
  XYZ::decorate_particle(p);
  p->add_attribute(internal::rigid_body_data().body_,
                   rb);
}

inline void add_required_attributes_for_body_member(Particle *p,
                                                    Particle* rb) {
  add_required_attributes_for_member(p, rb);
  for (unsigned int i=0; i< 4; ++i) {
    p->add_attribute(rigid_body_data().lquaternion_[i], 0);
  }
}

inline void remove_required_attributes_for_member(Particle *p) {
  for (unsigned int i=0; i< 3; ++i) {
    p->remove_attribute(rigid_body_data().child_keys_[i]);
  }
  p->remove_attribute(internal::rigid_body_data().body_);
}

inline void remove_required_attributes_for_body_member(Particle *p) {
  remove_required_attributes_for_member(p);
  for (unsigned int i=0; i< 4; ++i) {
    p->remove_attribute(rigid_body_data().lquaternion_[i]);
  }
}


IMPCORE_END_INTERNAL_NAMESPACE

#endif  /* IMPCORE_INTERNAL_RIGID_BODIES_H */
