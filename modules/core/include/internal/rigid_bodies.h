/**
 *  \file rigid_bodies.h
 *  \brief utilities for rigid bodies.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_INTERNAL_RIGID_BODIES_H
#define IMPCORE_INTERNAL_RIGID_BODIES_H

#include "../config.h"
#include "../XYZ.h"
#include "../Hierarchy.h"
#include <IMP/algebra/Sphere3D.h>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

struct RigidBodyData {
  FloatKeys child_keys_;
  FloatKeys quaternion_;
  HierarchyTraits htraits_;
  RigidBodyData(): htraits_("rigid_body") {
    child_keys_.resize(3);
    std::string pre="rigid_body_";
    child_keys_[0]= FloatKey((pre+"local_x").c_str());
    child_keys_[1]= FloatKey((pre+"local_y").c_str());
    child_keys_[2]= FloatKey((pre+"local_z").c_str());
    quaternion_.resize(4);
    quaternion_[0]= FloatKey((pre+"quaternion_0").c_str());
    quaternion_[1]= FloatKey((pre+"quaternion_1").c_str());
    quaternion_[2]= FloatKey((pre+"quaternion_2").c_str());
    quaternion_[3]= FloatKey((pre+"quaternion_3").c_str());
  }
};

IMPCOREEXPORT const RigidBodyData &rigid_body_data();


inline void set_model_ranges(Model *m) {
  m->set_range(rigid_body_data().quaternion_[0], FloatRange(0,1));
  m->set_range(rigid_body_data().quaternion_[1], FloatRange(0,1));
  m->set_range(rigid_body_data().quaternion_[2], FloatRange(0,1));
  m->set_range(rigid_body_data().quaternion_[3], FloatRange(0,1));
}

inline bool get_has_required_attributes_for_body(Particle *p) {
  IMP_check((p->has_attribute(rigid_body_data().quaternion_[0])
            && p->has_attribute(rigid_body_data().quaternion_[1])
            && p->has_attribute(rigid_body_data().quaternion_[2])
            && p->has_attribute(rigid_body_data().quaternion_[3])
             && XYZ::particle_is_instance(p))
            || (!p->has_attribute(rigid_body_data().quaternion_[0])
                && !p->has_attribute(rigid_body_data().quaternion_[1])
                && !p->has_attribute(rigid_body_data().quaternion_[2])
                && !p->has_attribute(rigid_body_data().quaternion_[3])),
            "Particle should have all of quaterion attributes or none",
            InvalidStateException);
  return p->has_attribute(rigid_body_data().quaternion_[0]);
}



inline bool
get_has_required_attributes_for_member(Particle *p) {
  for (unsigned int i=0; i< 3; ++i) {
    if (!p->has_attribute(rigid_body_data().child_keys_[i])) return false;
  }
  for (unsigned int i=0; i< 3; ++i) {
    if (!p->has_attribute(XYZ::get_coordinate_key(i)))
      return false;
  }
  return true;
}

inline void add_required_attributes_for_body(Particle *p) {
  for (unsigned int i=0; i< 4; ++i) {
    p->add_attribute(rigid_body_data().quaternion_[i], 0);
  }
  Hierarchy::setup_particle(p, rigid_body_data().htraits_);
  if (!XYZ::particle_is_instance(p)) {
      XYZ::setup_particle(p);
  }
}

inline void add_required_attributes_for_member(Particle *p) {
  for (unsigned int i=0; i< 3; ++i) {
    p->add_attribute(rigid_body_data().child_keys_[i], 0);
  }
  Hierarchy::setup_particle(p, rigid_body_data().htraits_);
  XYZ::decorate_particle(p);
}



IMPCORE_END_INTERNAL_NAMESPACE

#endif  /* IMPCORE_INTERNAL_RIGID_BODIES_H */
