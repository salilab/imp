/**
 *  \file rigid_bodies.cpp
 *  \brief Support for rigid bodies.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/core/rigid_body_geometries.h"
#include "IMP/core/XYZR.h"

IMPCORE_BEGIN_NAMESPACE

RigidBodyHierarchyGeometry::RigidBodyHierarchyGeometry(
    internal::RigidBodyHierarchy *h, unsigned int node, unsigned int layer)
    : display::SingletonGeometry(h->get_rigid_body()) {
  h_ = h;
  node_ = node;
  layer_ = layer;
}

RigidBodyHierarchyGeometry::RigidBodyHierarchyGeometry(
    RigidBody rb, const ParticlesTemp &constituents)
    : display::SingletonGeometry(rb) {
  h_ = internal::get_rigid_body_hierarchy(
      rb, IMP::internal::get_index(constituents), ObjectKey());
  node_ = 0;
  layer_ = 0;
}

display::Geometries RigidBodyHierarchyGeometry::get_components() const {
  display::Geometries ret;
  algebra::Sphere3D s = h_->get_sphere(node_);
  IMP_NEW(display::SphereGeometry, sg, (s));
  std::ostringstream oss;
  oss << h_->get_name() << " " << layer_;
  sg->set_name(oss.str());
  if (h_->get_is_leaf(node_)) {
    for (unsigned int i = 0; i < h_->get_number_of_particles(node_); ++i) {
      ret.push_back(
          new XYZRGeometry(XYZR(h_->get_model(), h_->get_particle(node_, i))));
    }
  } else {
    for (unsigned int i = 0; i < h_->get_number_of_children(node_); ++i) {
      ret.push_back(new RigidBodyHierarchyGeometry(h_, h_->get_child(node_, i),
                                                   layer_ + 1));
    }
  }
  return ret;
}

RigidBodyTorque::RigidBodyTorque(Particle *p)
    : display::SegmentGeometry(extract_geometry(p), p->get_name()), p_(p) {}
const algebra::Segment3D &RigidBodyTorque::get_geometry() const {
  cache_ = extract_geometry(p_);
  return cache_;
}

algebra::Segment3D RigidBodyTorque::extract_geometry(Particle *p) {
  RigidBody rb(p);
  algebra::Vector3D o = rb.get_coordinates();
  algebra::Vector3D ep = o + rb.get_torque();
  return algebra::Segment3D(o, ep);
}
IMPCORE_END_NAMESPACE
