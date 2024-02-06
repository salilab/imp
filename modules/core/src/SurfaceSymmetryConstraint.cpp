/**
 *  \file SurfaceSymmetryConstraint.cpp
 *  \brief Constrain orientation of surfaces with respect to rigid bodies.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/SurfaceSymmetryConstraint.h>
#include <IMP/algebra/Rotation3D.h>

IMPCORE_BEGIN_NAMESPACE

SurfaceSymmetryConstraint::SurfaceSymmetryConstraint(Model *m,
                          const ParticleIndexPair &ss,
                          const ParticleIndexPair &rbs)
  : IMP::Constraint(m, "SurfaceSymmetryConstraint%1%")
  , spip_(ss), rbpip_(rbs) {
    IMP_USAGE_CHECK(Surface::get_is_setup(get_model(), std::get<0>(ss)) &
                    Surface::get_is_setup(get_model(), std:get<1>(ss)),
                    "First pair must be Surface particles.");
    IMP_USAGE_CHECK(RigidBody::get_is_setup(get_model(), std::get<0>(rbs)) &
                    RigidBody::get_is_setup(get_model(), std::get<1>(rbs)),
                    "Second pair must be RigidBody particles.");
  }

algebra::Transformation3D SurfaceSymmetryConstraint::get_transformation() const {
  return algebra::get_transformation_from_first_to_second(
    RigidBody(get_model(), std::get<1>(rbpip_)).get_reference_frame(),
    RigidBody(get_model(), std::get<0>(rbpip_)).get_reference_frame());
}

void SurfaceSymmetryConstraint::do_update_attributes() {
  algebra::Transformation3D tf = get_transformation();
  Surface s, sref;
  s = Surface(get_model(), std::get<0>(spip_));
  sref = Surface(get_model(), std::get<1>(spip_));
  s.set_normal(tf.get_rotation().get_rotated(sref.get_normal()));
  s.set_coordinates(tf.get_transformed(sref.get_coordinates()));
}

void SurfaceSymmetryConstraint::do_update_derivatives(DerivativeAccumulator *da) {
  algebra::Rotation3D rot = get_transformation().get_inverse().get_rotation();
  Surface s1 = Surface(get_model(), std::get<0>(spip_));
  Surface s2 = Surface(get_model(), std::get<1>(spip_));
  s2.add_to_derivatives(rot.get_rotated(s1.get_derivatives()), *da);
  s2.add_to_normal_derivatives(rot.get_rotated(s1.get_normal_derivatives()),
                               *da);
}

ModelObjectsTemp SurfaceSymmetryConstraint::do_get_inputs() const {
  ParticlesTemp ps;
  ps.push_back(get_model()->get_particle(std::get<1>(spip_)));
  ps.push_back(get_model()->get_particle(std::get<0>(rbpip_)));
  ps.push_back(get_model()->get_particle(std::get<1>(rbpip_)));
  return ps;
}

ModelObjectsTemp SurfaceSymmetryConstraint::do_get_outputs() const {
  ParticlesTemp ps;
  ps.push_back(get_model()->get_particle(std::get<0>(spip_)));
  ps.push_back(get_model()->get_particle(std::get<1>(spip_)));
  return ps;
}

IMPCORE_END_NAMESPACE
