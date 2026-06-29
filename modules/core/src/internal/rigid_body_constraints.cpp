/**
 *  \file rigid_body_constraints.cpp
 *  \brief Constraints to keep rigid bodies rigid
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/internal/rigid_body_constraints.h>
#include <IMP/core/rigid_bodies.h>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

void _UpdateRigidBodyMembers::apply_index(Model *m,
                                          ParticleIndex pi) const {
  RigidBody rb(m, pi);
  rb.update_members();
}

ModelObjectsTemp _UpdateRigidBodyMembers::do_get_inputs(
    Model *m, const ParticleIndexes &pis) const {
  ModelObjectsTemp ret;
  ret += IMP::get_particles(m, pis);
  return ret;
}

ModelObjectsTemp _UpdateRigidBodyMembers::do_get_outputs(
    Model *m, const ParticleIndexes &pis) const {
  ModelObjectsTemp ret;
  for (unsigned int i = 0; i < pis.size(); ++i) {
    RigidBody rb(m, pis[i]);
    ret += IMP::get_particles(m, rb.get_member_particle_indexes());
    ret += IMP::get_particles(m, rb.get_body_member_particle_indexes());
  }
  return ret;
}

void _AccumulateRigidBodyDerivatives::apply_index(
    Model *m, ParticleIndex pi) const {
  IMP_OBJECT_LOG;
  DerivativeAccumulator da;
  RigidBody rb(m, pi);
#if IMP_HAS_CHECKS >= IMP_INTERNAL
  algebra::Vector4D oldderiv;
  algebra::Vector3D oldcartesian = rb.get_derivatives();
  for (unsigned int j = 0; j < 4; ++j) {
    oldderiv[j] = rb.get_particle()->get_derivative(
        internal::rigid_body_data().quaternion_[j]);
  }
#endif

  rb.pull_back_members_adjoints(da);

  IMP_LOG_TERSE("Rigid body derivative is "
                << m->get_particle(pi)->get_derivative(
                       internal::rigid_body_data().quaternion_[0]) << " "
                << m->get_particle(pi)->get_derivative(
                       internal::rigid_body_data().quaternion_[1]) << " "
                << m->get_particle(pi)->get_derivative(
                       internal::rigid_body_data().quaternion_[2]) << " "
                << m->get_particle(pi)->get_derivative(
                       internal::rigid_body_data().quaternion_[3]) << " and ");

  IMP_LOG_TERSE(
      "Translation deriv is " << static_cast<XYZ>(rb).get_derivatives() << ""
                              << std::endl);
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    algebra::Rotation3D rot =
        rb.get_reference_frame().get_transformation_to().get_rotation();
    // IMP_LOG_TERSE( "Accumulating rigid body derivatives" << std::endl);
    algebra::Vector3D v(0, 0, 0);
    Eigen::Vector4d q = Eigen::Vector4d::Zero();
    for (unsigned int i = 0; i < rb.get_number_of_members(); ++i) {
      RigidBodyMember d = rb.get_member(i);
      algebra::Vector3D dv = d.get_derivatives();
      v += dv;
      // IMP_LOG_TERSE( "Adding " << dv << " to derivative" << std::endl);
      q += rot.get_jacobian_of_rotated(Eigen::Vector3d(
        d.get_internal_coordinates().get_data()), false).transpose() *
        Eigen::Vector3d(dv.get_data());

      if (RigidBody::get_is_setup(d)) {
        algebra::Rotation3D mrot = RigidBodyMember(d).get_internal_transformation().get_rotation();
        Eigen::Vector4d mq(RigidBody(d).get_rotational_derivatives().get_data());
        Eigen::MatrixXd dq =
          algebra::get_jacobian_of_composed_wrt_first(rot, mrot, false).transpose();
        q += dq * mq;
      }
    }
    for (unsigned int j = 0; j < 4; ++j) {
#if IMP_HAS_CHECKS >= IMP_INTERNAL
      double d = rb.get_particle()->get_derivative(
                     internal::rigid_body_data().quaternion_[j]) -
                 oldderiv[j];
#endif
      IMP_INTERNAL_CHECK(std::abs(d - q[j]) < .05 * std::abs(d + q[j]) + .05,
                         "Derivatives do not match "
                             << oldderiv << ": "
                             << rb.get_particle()->get_derivative(
                                    internal::rigid_body_data().quaternion_[0])
                             << " "
                             << rb.get_particle()->get_derivative(
                                    internal::rigid_body_data().quaternion_[1])
                             << " "
                             << rb.get_particle()->get_derivative(
                                    internal::rigid_body_data().quaternion_[2])
                             << " "
                             << rb.get_particle()->get_derivative(
                                    internal::rigid_body_data().quaternion_[3])
                             << ": " << q);
    }
#if IMP_HAS_CHECKS >= IMP_INTERNAL
    algebra::Vector3D deltacartesian = rb.get_derivatives() - oldcartesian;
#endif
    IMP_INTERNAL_CHECK((deltacartesian - v).get_magnitude() <
                           .01 * (v + deltacartesian).get_magnitude() + .1,
                       "Cartesian derivatives don't match : " << deltacartesian
                                                              << " vs " << v);
  }
}

ModelObjectsTemp _AccumulateRigidBodyDerivatives::do_get_inputs(
    Model *m, const ParticleIndexes &pis) const {
  Refiner *refiner = internal::get_rigid_members_refiner();
  ModelObjectsTemp ret = refiner->get_inputs(m, pis);
  ret += IMP::get_particles(m, pis);
  for (unsigned int i = 0; i < pis.size(); ++i) {
    ret +=
        IMP::get_particles(m, refiner->get_refined_indexes(m, pis[i]));
  }
  return ret;
}

ModelObjectsTemp _AccumulateRigidBodyDerivatives::do_get_outputs(
    Model *m, const ParticleIndexes &pis) const {
  ModelObjectsTemp ret = IMP::get_particles(m, pis);
  return ret;
}

void _NormalizeRotation::apply_index(Model *m, ParticleIndex p) const {
  apply_indexes(m, ParticleIndexes(1,p), 0, 1);
}

void _NormalizeRotation::apply_indexes(
    Model *m, const ParticleIndexes &pis, unsigned int lower_bound,
    unsigned int upper_bound) const
{
  // direct access to tables for speed
  double* quaternion_tables[4];
  for(unsigned int i = 0; i < 4; i++){
    quaternion_tables[i]=
      core::RigidBody::access_quaternion_i_data(m, i);
  }
  for (unsigned int i = lower_bound; i < upper_bound; ++i) {
    int pi=pis[i].get_index();
    algebra::VectorD<4> v(quaternion_tables[0][pi],
                        quaternion_tables[1][pi],
                        quaternion_tables[2][pi],
                        quaternion_tables[3][pi]);
    IMP_LOG_TERSE( "Rotation quaternion before normalization: " << v << std::endl);
    double sm = v.get_squared_magnitude();
    if (sm < .0001) {
      IMP_LOG_TERSE("Near-zero rotation quaternion set to identity");
      quaternion_tables[0][pi] = 1;
      quaternion_tables[1][pi] = 0;
      quaternion_tables[2][pi] = 0;
      quaternion_tables[3][pi] = 0;
    } else if (std::abs(sm - 1.0) > .01) {
      double magnitude = std::sqrt(sm);
      quaternion_tables[0][pi] = v[0]/magnitude;
      quaternion_tables[1][pi] = v[1]/magnitude;
      quaternion_tables[2][pi] = v[2]/magnitude;
      quaternion_tables[3][pi] = v[3]/magnitude;
      IMP_LOG_TERSE( "Rotation quaternion normalized to " << v << std::endl);
    }
    IMP_INTERNAL_CHECK
      (std::abs(core::RigidBody(m,pis[i]).get_rotation()
                .get_quaternion().get_magnitude() - 1.0) < .01,
       "Quaternion expected to be normalized");
  }

  // evil hack - to reset all torques (BR: is it needed anywhere? for the attribute rather than the derivative? who ever used the torque attribute rather than derivative? it's supposedly angular momentum but it's never used anywhere this way, and why should it be reset anyway?)
  if(true){
    algebra::Vector3D* torque_table =
      core::RigidBody::access_torque_data(m);
    for (unsigned int j = lower_bound; j < upper_bound; j++) {
      torque_table[j] = algebra::Vector3D(0, 0, 0);
    }
  }
}

ModelObjectsTemp _NormalizeRotation::do_get_inputs(
    Model *m, const ParticleIndexes &pis) const {
  return IMP::get_particles(m, pis);
}

ModelObjectsTemp _NormalizeRotation::do_get_outputs(
    Model *m, const ParticleIndexes &pis) const {
  return IMP::get_particles(m, pis);
}

IMP_OBJECT_SERIALIZE_IMPL(IMP::core::internal::_RigidBodyNormalizeConstraint);
IMP_OBJECT_SERIALIZE_IMPL(IMP::core::internal::_RigidBodyPositionConstraint);

IMPCORE_END_INTERNAL_NAMESPACE
