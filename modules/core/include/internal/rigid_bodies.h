/**
 *  \file rigid_bodies.h
 *  \brief utilities for rigid bodies.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_INTERNAL_RIGID_BODIES_H
#define IMPCORE_INTERNAL_RIGID_BODIES_H

#include <IMP/core/core_config.h>
#include "../XYZ.h"
#include "../Hierarchy.h"
#include <IMP/algebra/Sphere3D.h>
namespace IMP {
namespace display {
class Geometry;
IMP_OBJECTS(Geometry, Geometries);
}
}

IMPCORE_BEGIN_INTERNAL_NAMESPACE

struct RigidBodyData {
  FloatKeys child_keys_;
  FloatKeys quaternion_;
  FloatKeys torque_;
  FloatKeys lquaternion_;
  IntKey is_rigid_key_;
  ParticleIndexesKey members_;
  ParticleIndexesKey body_members_;
  ParticleIndexKey body_;
  ObjectKey refkey_;
  RigidBodyData() {
    child_keys_.resize(3);
    std::string pre = "rigid_body_";
    // rigid body internal (local) coordinates are currently special cased
    child_keys_[0] = FloatKey(4);
    child_keys_[1] = FloatKey(5);
    child_keys_[2] = FloatKey(6);
    quaternion_.resize(4);
    quaternion_[0] = FloatKey((pre + "quaternion_0").c_str());
    quaternion_[1] = FloatKey((pre + "quaternion_1").c_str());
    quaternion_[2] = FloatKey((pre + "quaternion_2").c_str());
    quaternion_[3] = FloatKey((pre + "quaternion_3").c_str());
    is_rigid_key_ = IntKey(pre + "_is_rigid");
    torque_.resize(3);
    torque_[0] = FloatKey((pre + "torque_0").c_str());
    torque_[1] = FloatKey((pre + "torque_1").c_str());
    torque_[2] = FloatKey((pre + "torque_2").c_str());
    lquaternion_.resize(4);
    lquaternion_[0] = FloatKey((pre + "local_quaternion_0").c_str());
    lquaternion_[1] = FloatKey((pre + "local_quaternion_1").c_str());
    lquaternion_[2] = FloatKey((pre + "local_quaternion_2").c_str());
    lquaternion_[3] = FloatKey((pre + "local_quaternion_3").c_str());
    refkey_ = ObjectKey("rigid body representation");
    members_ = ParticleIndexesKey("rigid body members");
    body_members_ = ParticleIndexesKey("rigid body body members");
    body_ = ParticleIndexKey("rigid body");
  }
};

inline const RigidBodyData &rigid_body_data(){
  static const RigidBodyData rbd;
  return rbd;
}
;

inline void set_model_ranges(Model *m) {
  m->set_range(rigid_body_data().quaternion_[0], FloatRange(0, 1));
  m->set_range(rigid_body_data().quaternion_[1], FloatRange(0, 1));
  m->set_range(rigid_body_data().quaternion_[2], FloatRange(0, 1));
  m->set_range(rigid_body_data().quaternion_[3], FloatRange(0, 1));
}

inline bool get_has_required_attributes_for_body(
    Model *m, ParticleIndexAdaptor pi) {
  IMP_USAGE_CHECK(
      (m->get_has_attribute(rigid_body_data().quaternion_[0], pi) &&
       m->get_has_attribute(rigid_body_data().quaternion_[1], pi) &&
       m->get_has_attribute(rigid_body_data().quaternion_[2], pi) &&
       m->get_has_attribute(rigid_body_data().quaternion_[3], pi) &&
       XYZ::get_is_setup(m->get_particle(pi))) ||
          (!m->get_has_attribute(rigid_body_data().quaternion_[0], pi) &&
           !m->get_has_attribute(rigid_body_data().quaternion_[1], pi) &&
           !m->get_has_attribute(rigid_body_data().quaternion_[2], pi) &&
           !m->get_has_attribute(rigid_body_data().quaternion_[3], pi)),
      "Particle should have all of quaterion attributes or none");
  return m->get_has_attribute(rigid_body_data().quaternion_[0], pi);
}

inline bool get_has_required_attributes_for_member(
    Model *m, ParticleIndexAdaptor p) {
  if (!m->get_has_attribute(rigid_body_data().body_, p))
    return false;
  else {
    for (unsigned int i = 0; i < 3; ++i) {
      IMP_INTERNAL_CHECK(
          m->get_has_attribute(rigid_body_data().child_keys_[i], p),
          "Rigid member missing internal (local) coords");
    }
    IMP_INTERNAL_CHECK(XYZ::get_is_setup(m, p),
                       "Rigid member missing global coordinates");
    return true;
  }
}

inline bool get_has_required_attributes_for_rigid_member(
    Model *m, ParticleIndexAdaptor p) {
  if (!get_has_required_attributes_for_member(m, p)) return false;
  if (!m->get_has_attribute(rigid_body_data().is_rigid_key_, p)) return false;
  if (m->get_attribute(rigid_body_data().is_rigid_key_, p) != 1) return false;
  return true;
}

inline bool get_has_required_attributes_for_non_member(
    Model *m, ParticleIndexAdaptor p) {
  if (!get_has_required_attributes_for_member(m, p)) return false;
  if (!m->get_has_attribute(rigid_body_data().is_rigid_key_, p)) return false;
  if (m->get_attribute(rigid_body_data().is_rigid_key_, p) != 0) return false;
  return true;
}

inline bool get_has_required_attributes_for_body_member(
    Model *m, ParticleIndexAdaptor p) {
  if (!get_has_required_attributes_for_member(m, p)) return false;
  for (unsigned int i = 0; i < 4; ++i) {
    if (!m->get_has_attribute(rigid_body_data().lquaternion_[i], p)) {
      return false;
    }
  }
  return get_has_required_attributes_for_member(m, p);
}

inline void add_required_attributes_for_body(Model *m,
                                             ParticleIndexAdaptor p) {
  for (unsigned int i = 0; i < 4; ++i) {
    m->add_attribute(rigid_body_data().quaternion_[i], p, 0);
    m->set_range(rigid_body_data().quaternion_[i], FloatRange(0, 1));
  }
  for (unsigned int i = 0; i < 3; ++i) {
    m->add_attribute(rigid_body_data().torque_[i], p, 0);
  }
  if (!XYZ::get_is_setup(m, p)) {
    XYZ::setup_particle(m, p);
  }
}
inline void remove_required_attributes_for_body(
    Model *m, ParticleIndexAdaptor p) {
  for (unsigned int i = 0; i < 4; ++i) {
    m->remove_attribute(rigid_body_data().quaternion_[i], p);
  }
  for (unsigned int i = 0; i < 3; ++i) {
    m->remove_attribute(rigid_body_data().torque_[i], p);
  }
  if (m->get_has_attribute(internal::rigid_body_data().members_, p)) {
    m->remove_attribute(internal::rigid_body_data().members_, p);
  }
  if (m->get_has_attribute(internal::rigid_body_data().body_members_, p)) {
    m->remove_attribute(internal::rigid_body_data().body_members_, p);
  }
}

inline void add_required_attributes_for_member(
    Model *m, ParticleIndexAdaptor p,
    ParticleIndexAdaptor rb) {
  for (unsigned int i = 0; i < 3; ++i) {
    m->add_attribute(rigid_body_data().child_keys_[i], p, 0);
  }
  IMP_INTERNAL_CHECK(m->get_internal_coordinates(p).get_magnitude() < .01,
                     "Bad initialization");
  m->add_attribute(internal::rigid_body_data().body_, p, rb);
  m->add_attribute(internal::rigid_body_data().is_rigid_key_, p, 1);
}

inline void add_required_attributes_for_non_member(
    Model *m, ParticleIndexAdaptor p,
    ParticleIndexAdaptor rb) {
  for (unsigned int i = 0; i < 3; ++i) {
    m->add_attribute(rigid_body_data().child_keys_[i], p, 0);
  }
  IMP_INTERNAL_CHECK(m->get_internal_coordinates(p).get_magnitude() < .01,
                     "Bad initialization");
  m->add_attribute(internal::rigid_body_data().body_, p, rb);
  m->add_attribute(internal::rigid_body_data().is_rigid_key_, p, 0);
}

inline void add_required_attributes_for_body_member(
    Model *m, ParticleIndexAdaptor p,
    ParticleIndexAdaptor rb) {
  add_required_attributes_for_member(m, p, rb);
  for (unsigned int i = 0; i < 4; ++i) {
    m->add_attribute(rigid_body_data().lquaternion_[i], p, 0);
  }
  m->add_attribute(internal::rigid_body_data().is_rigid_key_, p, 1);
}

inline void remove_required_attributes_for_member(
    Model *m, ParticleIndexAdaptor p) {
  for (unsigned int i = 0; i < 3; ++i) {
    m->remove_attribute(rigid_body_data().child_keys_[i], p);
  }
  m->remove_attribute(internal::rigid_body_data().body_, p);
  m->remove_attribute(internal::rigid_body_data().is_rigid_key_, p);
}

inline void remove_required_attributes_for_body_member(
    Model *m, ParticleIndexAdaptor p) {
  remove_required_attributes_for_member(m, p);
  for (unsigned int i = 0; i < 4; ++i) {
    m->remove_attribute(rigid_body_data().lquaternion_[i], p);
  }
}

IMPCOREEXPORT display::Geometries get_rigid_body_derivative_geometries(
    Model *m, ParticleIndex pi);

IMPCORE_END_INTERNAL_NAMESPACE

#endif /* IMPCORE_INTERNAL_RIGID_BODIES_H */
