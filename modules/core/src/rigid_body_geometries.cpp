/**
 *  \file rigid_bodies.cpp
 *  \brief Support for rigid bodies.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/core/rigid_body_geometries.h"
#include "IMP/core/XYZR.h"

IMPCORE_BEGIN_NAMESPACE
namespace internal {

display::Geometries get_rigid_body_derivative_geometries(
    kernel::Model *m, kernel::ParticleIndex pi) {
  RigidBody d(m, pi);
  display::Geometries ret;
  kernel::Particles ms = get_as<kernel::Particles>(d.get_members());
  algebra::Transformation3D otr =
      d.get_reference_frame().get_transformation_to();
  algebra::VectorD<4> rderiv = d.get_rotational_derivatives();
  algebra::Vector3D tderiv = d.get_derivatives();
  algebra::VectorD<4> rot = otr.get_rotation().get_quaternion();
  IMP_LOG_TERSE("Old rotation was " << rot << std::endl);
  Float scale = .1;
  algebra::VectorD<4> dv = rderiv;
  if (dv.get_squared_magnitude() > 0.00001) {
    dv = scale * dv.get_unit_vector();
  }
  rot += dv;
  rot = rot.get_unit_vector();
  algebra::Rotation3D r(rot[0], rot[1], rot[2], rot[3]);
  IMP_LOG_TERSE("Derivative was " << tderiv << std::endl);
  IMP_LOG_TERSE("New rotation is " << rot << std::endl);
  FloatRange xr =
      d.get_particle()->get_model()->get_range(core::XYZ::get_xyz_keys()[0]);
  Float wid = xr.second - xr.first;
  algebra::Vector3D stderiv = scale * tderiv * wid;
  algebra::Transformation3D ntr(
      algebra::Rotation3D(rot[0], rot[1], rot[2], rot[3]),
      stderiv + otr.get_translation());
  for (unsigned int i = 0; i < ms.size(); ++i) {
    core::RigidMember dm(ms[i]);
    display::SegmentGeometry *tr = new display::SegmentGeometry(
        algebra::Segment3D(dm.get_coordinates(), dm.get_coordinates() + tderiv),
        /*xyzcolor_*/
        display::Color(1, 0, 0));
    ret.push_back(tr);
    algebra::Vector3D ic =
        r.get_rotated(dm.get_internal_coordinates()) + d.get_coordinates();
    display::SegmentGeometry *rtr = new display::SegmentGeometry(
        algebra::Segment3D(dm.get_coordinates(), ic), display::Color(0, 1, 0));
    ret.push_back(rtr);
    display::SegmentGeometry *nrtr = new display::SegmentGeometry(
        algebra::Segment3D(dm.get_coordinates(),
                           ntr.get_transformed(dm.get_internal_coordinates())),
        display::Color(0, 0, 1));
    ret.push_back(nrtr);
  }
  return ret;
}
}

RigidBodyHierarchyGeometry::RigidBodyHierarchyGeometry(
    internal::RigidBodyHierarchy *h, unsigned int node, unsigned int layer)
    : display::SingletonGeometry(h->get_rigid_body()) {
  h_ = h;
  node_ = node;
  layer_ = layer;
}

RigidBodyHierarchyGeometry::RigidBodyHierarchyGeometry(
    RigidBody rb, const kernel::ParticlesTemp &constituents)
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

RigidBodyTorque::RigidBodyTorque(kernel::Particle *p)
    : display::SegmentGeometry(extract_geometry(p), p->get_name()), p_(p) {}
const algebra::Segment3D &RigidBodyTorque::get_geometry() const {
  cache_ = extract_geometry(p_);
  return cache_;
}

algebra::Segment3D RigidBodyTorque::extract_geometry(kernel::Particle *p) {
  RigidBody rb(p);
  algebra::Vector3D o = rb.get_coordinates();
  algebra::Vector3D ep = o + rb.get_torque();
  return algebra::Segment3D(o, ep);
}
IMPCORE_END_NAMESPACE
