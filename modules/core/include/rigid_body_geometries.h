/**
 *  \file IMP/core/rigid_body_geometries.h
 *  \brief functionality for defining rigid bodies
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_RIGID_BODY_GEOMETRIES_H
#define IMPCORE_RIGID_BODY_GEOMETRIES_H

#include <IMP/core/core_config.h>
#include "rigid_bodies.h"
#include "internal/rigid_body_tree.h"
#include <IMP/display/particle_geometry.h>

IMPCORE_BEGIN_NAMESPACE

/** Show the collision detection hierarchy for a rigid body.*/
class IMPCOREEXPORT RigidBodyHierarchyGeometry
    : public display::SingletonGeometry {
  base::Pointer<internal::RigidBodyHierarchy> h_;
  unsigned int node_, layer_;
  RigidBodyHierarchyGeometry(internal::RigidBodyHierarchy *h, unsigned int node,
                             unsigned int layer);

 public:
  RigidBodyHierarchyGeometry(RigidBody rb,
                             const kernel::ParticlesTemp &constituents);
  display::Geometries get_components() const;
  IMP_OBJECT_METHODS(RigidBodyHierarchyGeometry);
};

IMP_PARTICLE_GEOMETRY(RigidBodyDerivative, core::RigidBody, {
  ret = internal::get_rigid_body_derivative_geometries(d.get_model(),
                                                       d.get_particle_index());
});

IMP_PARTICLE_GEOMETRY(RigidBodyFrame, core::RigidBody, {
  ret.push_back(new display::ReferenceFrameGeometry(d.get_reference_frame()));
});

/** Display the torque on a rigid body as a line segment.*/
class IMPCOREEXPORT RigidBodyTorque : public display::SegmentGeometry {
  base::WeakPointer<kernel::Particle> p_;
  mutable algebra::Segment3D cache_;
  static algebra::Segment3D extract_geometry(kernel::Particle *p);

 public:
  RigidBodyTorque(kernel::Particle *p);
  const algebra::Segment3D &get_geometry() const;
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_RIGID_BODY_GEOMETRIES_H */
