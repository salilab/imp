/**
 *  \file rigid_bodies.h
 *  \brief utilities for rigid bodies.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_INTERNAL_RIGID_BODIES_H
#define IMPCORE_INTERNAL_RIGID_BODIES_H

#include "../config.h"
#include "../XYZDecorator.h"
#include "../HierarchyDecorator.h"
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
  for (unsigned int i=0; i< 4; ++i) {
      if (!p->has_attribute(rigid_body_data().quaternion_[i])) return false;
  }
  for (unsigned int i=0; i< 3; ++i) {
    if (!p->has_attribute(XYZDecorator::get_coordinate_key(i)))
      return false;
  }
  return true;
}



inline bool
get_has_required_attributes_for_member(Particle *p) {
  for (unsigned int i=0; i< 3; ++i) {
    if (!p->has_attribute(rigid_body_data().child_keys_[i])) return false;
  }
  for (unsigned int i=0; i< 3; ++i) {
    if (!p->has_attribute(XYZDecorator::get_coordinate_key(i)))
      return false;
  }
  return true;
}

inline void add_required_attributes_for_body(Particle *p) {
  for (unsigned int i=0; i< 4; ++i) {
    p->add_attribute(rigid_body_data().quaternion_[i], 0);
  }
  HierarchyDecorator::create(p, rigid_body_data().htraits_);
  if (!XYZDecorator::is_instance_of(p)) {
      XYZDecorator::create(p);
  }
}

inline void add_required_attributes_for_member(Particle *p) {
  for (unsigned int i=0; i< 3; ++i) {
    p->add_attribute(rigid_body_data().child_keys_[i], 0);
  }
  HierarchyDecorator::create(p, rigid_body_data().htraits_);
  XYZDecorator::cast(p);
}


class IMPCOREEXPORT RigidBodyParticleData {
  struct Data {
    std::vector<int> storage_;
    algebra::Sphere3D s_;
  };
  std::vector<Data> data_;
public:
  RigidBodyParticleData();
  void set_sphere(unsigned int ni, const algebra::Sphere3D &s);
  void set_leaf(unsigned int ni, const std::vector<unsigned int> &ids);
  unsigned int add_children(unsigned int ni, unsigned int num_children);
  bool get_is_leaf(unsigned int ni) const;
  unsigned int get_number_of_particles(unsigned int ni) const;
  unsigned int get_number_of_children(unsigned int ni) const;
  unsigned int get_child(unsigned int ni, unsigned int i) const;
  unsigned int get_particle(unsigned int ni, unsigned int i) const;
  const algebra::Sphere3D &get_sphere(unsigned int ni) const;
  void show_tree(std::ostream &out) const;
};


typedef std::vector<unsigned int> SphereIndexes;
typedef std::vector<SphereIndexes> SpheresSplit;


class IMPCOREEXPORT RigidBodyCollisionData {
  std::map<Particle*, RigidBodyParticleData> data_;
public:
  // for a non rigid body
  void add_data(Particle *p, const algebra::Sphere3D &s);
  void add_data(Particle *p);
  RigidBodyParticleData &get_data(Particle *p);
  const RigidBodyParticleData &get_data(Particle *p) const;
  bool has_data(Particle *p) const;
  void show(std::ostream &out) const;
};


IMPCORE_END_INTERNAL_NAMESPACE

#endif  /* IMPCORE_INTERNAL_RIGID_BODIES_H */
