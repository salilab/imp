/**
 *  \file pmi/InternalCoordinatesConstraint.cpp
 *  \brief A singleton modifier which wraps an attribute into a
 *  given range.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/pmi/InternalCoordinatesConstraint.h"
#include "IMP/core/symmetry.h"
#include "IMP/core/rigid_bodies.h"
#include "IMP/algebra/Vector3D.h"

IMPPMI_BEGIN_NAMESPACE
InternalCoordinatesConstraint::InternalCoordinatesConstraint() {}

void InternalCoordinatesConstraint::apply_index(Model *m,
                                           ParticleIndex pi) const {

    IMP::ParticleIndex ref_index=IMP::core::Reference(m,pi).get_reference_particle()->get_index();
    IMP::core::NonRigidMember nrm1=IMP::core::NonRigidMember(m,ref_index);
    IMP::core::NonRigidMember nrm2=IMP::core::NonRigidMember(m,pi);
    IMP::algebra::Vector3D c1=nrm1.get_internal_coordinates();
    nrm2.set_internal_coordinates(c1);
}

/* Only the passed particle is used */
ModelObjectsTemp InternalCoordinatesConstraint::do_get_inputs(
    Model *m, const ParticleIndexes &pis) const {
  return IMP::get_particles(m, pis);
}

ModelObjectsTemp InternalCoordinatesConstraint::do_get_outputs(
    Model *m, const ParticleIndexes &pis) const {
  return do_get_inputs(m, pis);
}

IMPPMI_END_NAMESPACE
