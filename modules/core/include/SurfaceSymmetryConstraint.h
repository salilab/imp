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
#include <IMP/algebra/Transformation3D.h>
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

  algebra::Transformation3D get_transformation() const;

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
                              const ParticleIndexPair &rbs);

    virtual void do_update_attributes() IMP_OVERRIDE;

    virtual void do_update_derivatives(DerivativeAccumulator *da) IMP_OVERRIDE;

    virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;

    virtual ModelObjectsTemp do_get_outputs() const IMP_OVERRIDE;

   IMP_OBJECT_METHODS(SurfaceSymmetryConstraint);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_SURFACE_SYMMETRY_CONSTRAINT_H */
