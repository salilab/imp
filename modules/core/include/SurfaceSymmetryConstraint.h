/**
 *  \file IMP/core/SurfaceSymmetryConstraint.h
 *  \brief Constrain orientation of surfaces with respect to rigid bodies.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_SURFACE_SYMMETRY_CONSTRAINT_H
#define IMPCORE_SURFACE_SYMMETRY_CONSTRAINT_H

#include <IMP/core/core_config.h>
#include <IMP/core/Surface.h>
#include <IMP/core/rigid_bodies.h>
#include <IMP/algebra/Rotation3D.h>
#include <IMP/algebra/ReferenceFrame3D.h>
#include <IMP/Constraint.h>
#include <IMP/Particle.h>
#include <IMP/Object.h>
#include <IMP/Model.h>


IMPCORE_BEGIN_NAMESPACE

//! Constrain orientation of surfaces with respect to rigid bodies.
/** Given two surfaces and two rigid bodies assumed to be identical,
    constrain the orientation of the first surface relative to the
    first rigid body to be the same as the orientation of second
    surface with respect to the second rigid body.

    \note Both RigidBody particles are assumed to be identical copies.
    This is not enforced.

    \note Derivatives from the first surface are transformed and
    added to the second surface.
*/
class IMPCOREEXPORT SurfaceSymmetryConstraint : public IMP::Constraint {
  ParticleIndexPair spip_;
  ParticleIndexPair rbpip_;

  algebra::Transformation3D get_transformation() const {
    return algebra::get_transformation_from_first_to_second(
      RigidBody(get_model(), rbpip_[1]).get_reference_frame(),
      RigidBody(get_model(), rbpip_[0]).get_reference_frame());
  }

  public:
    //! Create restraint from particle pairs.
    /** \param[in] m the Model
        \param[in] ss pair of Surface particles, where the first surface is
                   constrained
        \param[in] rbs pair of RigidBody particles, corresponding to
                   the surfaces in `ss`
     */
    SurfaceSymmetryConstraint(Model *m,
                              const ParticleIndexPair &ss,
                              const ParticleIndexPair &rbs)
      : IMP::Constraint(m, "SurfaceSymmetryConstraint%1%")
      , spip_(ss), rbpip_(rbs) {
        IMP_USAGE_CHECK(Surface::get_is_setup(get_model(), ss[0]) &
                        Surface::get_is_setup(get_model(), ss[1]),
                        "First pair must be Surface particles.");
        IMP_USAGE_CHECK(RigidBody::get_is_setup(get_model(), rbs[0]) &
                        RigidBody::get_is_setup(get_model(), rbs[1]),
                        "Second pair must be RigidBody particles.");
      }

    virtual void do_update_attributes() IMP_OVERRIDE {
      algebra::Transformation3D tf = get_transformation();
      Surface s, sref;
      s = Surface(get_model(), spip_[0]);
      sref = Surface(get_model(), spip_[1]);
      s.set_normal(tf.get_rotation().get_rotated(sref.get_normal()));
      s.set_coordinates(tf.get_transformed(sref.get_coordinates()));
    }

    virtual void do_update_derivatives(DerivativeAccumulator *da) IMP_OVERRIDE {
      algebra::Rotation3D rot = get_transformation().get_inverse().get_rotation();
      Surface s1 = Surface(get_model(), spip_[0]);
      Surface s2 = Surface(get_model(), spip_[1]);
      s2.add_to_derivatives(rot.get_rotated(s1.get_derivatives()), *da);
      s2.add_to_normal_derivatives(
        rot.get_rotated(s1.get_normal_derivatives()), *da);
    }

    virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE {
      ParticlesTemp ps;
      ps.push_back(get_model()->get_particle(spip_[1]));
      ps.push_back(get_model()->get_particle(rbpip_[0]));
      ps.push_back(get_model()->get_particle(rbpip_[1]));
      return ps;
    }

    virtual ModelObjectsTemp do_get_outputs() const IMP_OVERRIDE {
      ParticlesTemp ps;
      ps.push_back(get_model()->get_particle(spip_[0]));
      ps.push_back(get_model()->get_particle(spip_[1]));
      return ps;
    }

   IMP_OBJECT_METHODS(SurfaceSymmetryConstraint);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_SURFACE_SYMMETRY_CONSTRAINT_H */
