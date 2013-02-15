/**
 *  \file IMP/core/rigid_body_geometries.h
 *  \brief functionality for defining rigid bodies
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_RIGID_BODY_GEOMETRIES_H
#define IMPCORE_RIGID_BODY_GEOMETRIES_H

#include <IMP/core/core_config.h>
#include "rigid_bodies.h"
#include "internal/rigid_body_tree.h"
#include <IMP/display/particle_geometry.h>

IMPCORE_BEGIN_NAMESPACE


/** Show the collision detection hierarchy for a rigid body.*/
class IMPCOREEXPORT RigidBodyHierarchyGeometry:
    public display::SingletonGeometry {
  Pointer<internal::RigidBodyHierarchy> h_;
  unsigned int node_, layer_;
  RigidBodyHierarchyGeometry(internal::RigidBodyHierarchy *h,
                             unsigned int node,
                             unsigned int layer);
public:
  RigidBodyHierarchyGeometry(RigidBody rb,
                             const ParticlesTemp &constituents);
  display::Geometries get_components() const;
  IMP_OBJECT_INLINE(RigidBodyHierarchyGeometry,IMP_UNUSED(out),);
};

IMP_PARTICLE_GEOMETRY(RigidBodyDerivative, core::RigidBody, {
    Particles ms=get_as<Particles>(d.get_members());
    algebra::Transformation3D otr
      = d.get_reference_frame().get_transformation_to();
    algebra::VectorD<4> rderiv= d.get_rotational_derivatives();
    algebra::Vector3D tderiv= d.get_derivatives();
    algebra::VectorD<4> rot = otr.get_rotation().get_quaternion();
    IMP_LOG_TERSE( "Old rotation was " << rot << std::endl);
    Float scale=.1;
    algebra::VectorD<4> dv=rderiv;
    if (dv.get_squared_magnitude() > 0.00001) {
      dv= scale*dv.get_unit_vector();
    }
    rot+= dv;
    rot= rot.get_unit_vector();
    algebra::Rotation3D r(rot[0], rot[1], rot[2], rot[3]);
    IMP_LOG_TERSE( "Derivative was " << tderiv << std::endl);
    IMP_LOG_TERSE( "New rotation is " << rot << std::endl);
    FloatRange xr= d.get_particle()->get_model()
      ->get_range(core::XYZ::get_xyz_keys()[0]);
    Float wid= xr.second-xr.first;
    algebra::Vector3D stderiv= scale*tderiv*wid;
    algebra::Transformation3D ntr(algebra::Rotation3D(rot[0], rot[1],
                                                      rot[2], rot[3]),
                                  stderiv+otr.get_translation());
    for (unsigned int i=0; i< ms.size(); ++i) {
      core::RigidMember dm(ms[i]);
      display::SegmentGeometry *tr
        = new display::SegmentGeometry(algebra::Segment3D(dm.get_coordinates(),
                                                 dm.get_coordinates()+tderiv),
                              /*xyzcolor_*/
                                       display::Color(1,0,0));
      ret.push_back(tr);
      algebra::Vector3D ic= r.get_rotated(dm.get_internal_coordinates())
      + d.get_coordinates();
      display::SegmentGeometry *rtr
        = new display::SegmentGeometry(algebra::Segment3D(dm.get_coordinates(),
                                                 ic),
                              display::Color(0,1,0));
      ret.push_back(rtr);
      display::SegmentGeometry *nrtr
         = new display::SegmentGeometry(algebra::Segment3D(dm.get_coordinates(),
                      ntr.get_transformed(dm.get_internal_coordinates())),
                              display::Color(0,0,1));
      ret.push_back(nrtr);
    }
  });

IMP_PARTICLE_GEOMETRY(RigidBodyFrame, core::RigidBody, {
    ret.push_back(new display::ReferenceFrameGeometry(d.get_reference_frame()));
  });

/** Display the torque on a rigid body as a line segment.*/
class IMPCOREEXPORT RigidBodyTorque: public display::SegmentGeometry {
  base::WeakPointer<Particle> p_;
  mutable algebra::Segment3D cache_;
  static algebra::Segment3D extract_geometry(Particle *p);
public:
  RigidBodyTorque(Particle *p);
  const algebra::Segment3D &get_geometry() const;
};


IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_RIGID_BODY_GEOMETRIES_H */
