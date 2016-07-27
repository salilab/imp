/**
 *  \file rigid_bodies.cpp
 *  \brief Support for rigid bodies.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/core/rigid_bodies.h"
#include "IMP/core/SingletonConstraint.h"
#include "IMP/generic.h"
#include <IMP/SingletonContainer.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/geometric_alignment.h>
#include <IMP/base_statistics.h>
#include <IMP/core/FixedRefiner.h>
#include <IMP/core/internal/rigid_body_tree.h>
#include <IMP/internal/ContainerConstraint.h>
#include <IMP/internal/StaticListContainer.h>
#include <IMP/internal/utility.h>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

IMPCORE_END_INTERNAL_NAMESPACE
IMPCORE_BEGIN_NAMESPACE
void RigidBody::normalize_rotation() {
  double &q0 = get_model()->access_attribute(
      internal::rigid_body_data().quaternion_[0], get_particle_index());
  double &q1 = get_model()->access_attribute(
      internal::rigid_body_data().quaternion_[1], get_particle_index());
  double &q2 = get_model()->access_attribute(
      internal::rigid_body_data().quaternion_[2], get_particle_index());
  double &q3 = get_model()->access_attribute(
      internal::rigid_body_data().quaternion_[3], get_particle_index());
  algebra::VectorD<4> v(q0, q1, q2, q3);
  // IMP_LOG_TERSE( "Rotation was " << v << std::endl);
  double sm = v.get_squared_magnitude();
  if (sm < .001) {
    //    v = algebra::VectorD<4>(1, 0, 0, 0);
    // IMP_LOG_TERSE( "Rotation is " << v << std::endl);
    q0 = 1;
    q1 = 0;
    q2 = 0;
    q3 = 0;
  } else if (std::abs(sm - 1.0) > .01) {
    v = v.get_unit_vector();
    // IMP_LOG_TERSE( "Rotation is " << v << std::endl);
    q0 = v[0];
    q1 = v[1];
    q2 = v[2];
    q3 = v[3];
  }

  if(true){ // TODO: BR - the attribute of torque is never used is it? MAKE SURE!!! this is actually angular momentum (and I don't know that it's ever used in imp this way - I think it is called velocity in MD module.
    // evil hack
    get_model()->set_attribute(internal::rigid_body_data().torque_[0],
                               get_particle_index(), 0);
    get_model()->set_attribute(internal::rigid_body_data().torque_[1],
                               get_particle_index(), 0);
    get_model()->set_attribute(internal::rigid_body_data().torque_[2],
                               get_particle_index(), 0);
  }
}

namespace {
ObjectKeys cache_keys;
}
void add_rigid_body_cache_key(ObjectKey k) {
  if (!std::binary_search(cache_keys.begin(), cache_keys.end(), k)) {
    cache_keys.push_back(k);
    std::sort(cache_keys.begin(), cache_keys.end());
  }
}

namespace {

/*ParticlesTemp get_rigid_body_used_particles(Particle *p) {
  RigidBody b(p);
  unsigned int n=b.get_number_of_members();
  ParticlesTemp ret(1+n);
  ret[0]=p;
  for (unsigned int i=0; i< n; ++i) {
    ret[i+1]= b.get_member(i);
  }
  return ret;
  }*/

//! Accumulate the derivatives from the refined particles in the rigid body
/** You can
    use the setup_rigid_bodies and setup_rigid_body methods instead of
    creating these objects yourself.
    \see setup_rigid_bodies
    \see setup_rigid_body
    \see RigidBody
    \verbinclude rigid_bodies.py
    \see UpdateRigidBodyMembers
*/
class AccumulateRigidBodyDerivatives : public SingletonDerivativeModifier {
 public:
  AccumulateRigidBodyDerivatives(std::string name =
                                     "AccumulateRigidBodyDerivatives%1%")
      : SingletonDerivativeModifier(name) {}
  virtual void apply_index(Model *m, ParticleIndex pi) const
      IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_outputs(
      Model *m, const ParticleIndexes &pis) const IMP_OVERRIDE;
  IMP_SINGLETON_MODIFIER_METHODS(AccumulateRigidBodyDerivatives);
  IMP_OBJECT_METHODS(AccumulateRigidBodyDerivatives);
};

/** \brief Compute the coordinates of the RigidMember objects bases
      on the orientation.

      This should be applied after evaluate to keep the bodies
      rigid. You can use the setup_rigid_bodies and setup_rigid_body
      methods instead of creating these objects yourself.

      \see setup_rigid_bodies
      \see setup_rigid_body
      \see RigidBody
      \see AccumulateRigidBodyDerivatives */
class UpdateRigidBodyMembers : public SingletonModifier {
 public:
  UpdateRigidBodyMembers(std::string name = "UpdateRigidBodyMembers%1%")
      : SingletonModifier(name) {}
  virtual void apply_index(Model *m, ParticleIndex pi) const
      IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_outputs(
      Model *m, const ParticleIndexes &pis) const IMP_OVERRIDE;
  IMP_SINGLETON_MODIFIER_METHODS(UpdateRigidBodyMembers);
  IMP_OBJECT_METHODS(UpdateRigidBodyMembers);
};

/** \brief Fix the normalization of the rotation term. */
class NormalizeRotation : public SingletonModifier {
 public:
  NormalizeRotation(std::string name = "NormalizeRotation%1%")
      : SingletonModifier(name) {}
  virtual void apply_index(Model *m, ParticleIndex pi) const
      IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_outputs(
      Model *m, const ParticleIndexes &pis) const IMP_OVERRIDE;
  virtual void apply_indexes(Model *m, const ParticleIndexes &pis,
                             unsigned int lower_bound,
                             unsigned int upper_bound) const IMP_FINAL;
  //  IMP_SINGLETON_MODIFIER_METHODS(NormalizeRotation);
  IMP_OBJECT_METHODS(NormalizeRotation);
};

/** \brief Fix the normalization of the rotation term. */
class NullSDM : public SingletonDerivativeModifier {
 public:
  NullSDM(std::string name = "NullModifier%1%")
      : SingletonDerivativeModifier(name) {}
  virtual void apply_index(Model *m, ParticleIndex pi) const
      IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_outputs(
      Model *m, const ParticleIndexes &pis) const IMP_OVERRIDE;
  IMP_SINGLETON_MODIFIER_METHODS(NullSDM);
  IMP_OBJECT_METHODS(NullSDM);
};

void AccumulateRigidBodyDerivatives::apply_index(
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
  algebra::Rotation3D rot =
      rb.get_reference_frame().get_transformation_from().get_rotation();
  algebra::Rotation3D roti =
      rb.get_reference_frame().get_transformation_to().get_rotation();
  const ParticleIndexes &rbis = rb.get_member_particle_indexes();
  for (unsigned int i = 0; i < rbis.size(); ++i) {
    RigidMember d(rb.get_model(), rbis[i]);
    const algebra::Vector3D &deriv = d.get_derivatives();
    if (deriv.get_squared_magnitude() > 0) {
      algebra::Vector3D dv = rot * deriv;
      rb.add_to_derivatives(dv, deriv, d.get_internal_coordinates(), roti, da);
    }
  }
  const ParticleIndexes &rbbis = rb.get_body_member_particle_indexes();
  for (unsigned int i = 0; i < rbbis.size(); ++i) {
    RigidMember d(rb.get_model(), rbbis[i]);
    const algebra::Vector3D &deriv = d.get_derivatives();
    if (deriv.get_squared_magnitude() > 0) {
      algebra::Vector3D dv = rot * deriv;
      rb.add_to_derivatives(dv, deriv, d.get_internal_coordinates(), roti, da);
    }
  }
  // ignoring torques on rigid members
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
    algebra::VectorD<4> q(0, 0, 0, 0);
    for (unsigned int i = 0; i < rb.get_number_of_members(); ++i) {
      RigidMember d = rb.get_member(i);
      algebra::Vector3D dv = d.get_derivatives();
      v += dv;
      // IMP_LOG_TERSE( "Adding " << dv << " to derivative" << std::endl);
      for (unsigned int j = 0; j < 4; ++j) {
        algebra::Vector3D v =
            rot.get_derivative(d.get_internal_coordinates(), j);
        /*IMP_LOG_VERBOSE( "Adding " << dv*v << " to quaternion deriv " << j
          << std::endl);*/
        q[j] += dv * v;
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
                                    internal::rigid_body_data().quaternion_[1])
                             << " "
                             << rb.get_particle()->get_derivative(
                                    internal::rigid_body_data().quaternion_[2])
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

ModelObjectsTemp AccumulateRigidBodyDerivatives::do_get_inputs(
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
ModelObjectsTemp AccumulateRigidBodyDerivatives::do_get_outputs(
    Model *m, const ParticleIndexes &pis) const {
  ModelObjectsTemp ret = IMP::get_particles(m, pis);
  return ret;
}

void UpdateRigidBodyMembers::apply_index(Model *m,
                                         ParticleIndex pi) const {
  RigidBody rb(m, pi);
  rb.update_members();
}
ModelObjectsTemp UpdateRigidBodyMembers::do_get_inputs(
    Model *m, const ParticleIndexes &pis) const {
  ModelObjectsTemp ret;
  ret += IMP::get_particles(m, pis);
  return ret;
}
ModelObjectsTemp UpdateRigidBodyMembers::do_get_outputs(
    Model *m, const ParticleIndexes &pis) const {
  ModelObjectsTemp ret;
  for (unsigned int i = 0; i < pis.size(); ++i) {
    RigidBody rb(m, pis[i]);
    ret += IMP::get_particles(m, rb.get_member_particle_indexes());
    ret += IMP::get_particles(m, rb.get_body_member_particle_indexes());
  }
  return ret;
}

inline void NormalizeRotation::apply_index(Model *m,
                                           ParticleIndex p) const {
  apply_indexes(m, ParticleIndexes(1,p), 0, 1);
}

inline void
NormalizeRotation::apply_indexes
(Model *m, const ParticleIndexes &pis,
 unsigned int lower_bound,
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
    bool is_update(false);
    if (sm < .0001) {
      IMP_LOG_TERSE('Near-zero rotation quaternion set to identity');
      quaternion_tables[0][pi] = 1;
      quaternion_tables[1][pi] = 0;
      quaternion_tables[2][pi] = 0;
      quaternion_tables[3][pi] = 0;
    } else if (std::abs(sm - 1.0) > .01) {
      double m=std::sqrt(sm); // magnitude
      quaternion_tables[0][pi] = v[0]/m;
      quaternion_tables[1][pi] = v[1]/m;
      quaternion_tables[2][pi] = v[2]/m;
      quaternion_tables[3][pi] = v[3]/m;
      IMP_LOG_TERSE( "Rotation quaternion normalized to " << v << std::endl);
    }
    IMP_INTERNAL_CHECK
      (std::abs(core::RigidBody(m,pis[i]).get_rotation()
                .get_quaternion().get_magnitude() - 1.0) < .01,
       "Quaternion expected to be normalized");
  }

  // evil hack - to reset all torques (BR: is it needed anywhere? for the attribute rather than the derivative? who ever used the torque attribute rather than derivative? it's supposedly angular momentum but it's never used anywhere this way, and why should it be reset anyway?)
  if(true){
    for(unsigned int i = 0; i < 3; i++){
      double* torque_table_i=
        core::RigidBody::access_torque_i_data(m, i);
      for (unsigned int j = lower_bound; j < upper_bound; j++) {
        torque_table_i[j]=0;
      } // for j
    } // for i
    //      ParticleIndex pi_j=pis[j];
    // m->set_attribute(internal::rigid_body_data().torque_[0], pi_j, 0);
    //m->set_attribute(internal::rigid_body_data().torque_[1], pi_j, 0);
    // m->set_attribute(internal::rigid_body_data().torque_[2], pi_j, 0);
    //  } // for j
  }
}


ModelObjectsTemp NormalizeRotation::do_get_inputs(
    Model *m, const ParticleIndexes &pis) const {
  return IMP::get_particles(m, pis);
}
ModelObjectsTemp NormalizeRotation::do_get_outputs(
    Model *m, const ParticleIndexes &pis) const {
  return IMP::get_particles(m, pis);
}

inline void NullSDM::apply_index(Model *, ParticleIndex) const {
}
ModelObjectsTemp NullSDM::do_get_inputs(Model *,
                                        const ParticleIndexes &) const {
  return ModelObjectsTemp();
}
ModelObjectsTemp NullSDM::do_get_outputs(
    Model *, const ParticleIndexes &) const {
  return ModelObjectsTemp();
}

ObjectKey get_rb_score_state_0_key() {
  static ObjectKey key("rigid body score state 0");
  return key;
}

ModelKey get_rb_list_key() {
  static ModelKey key("rigid body list");
  return key;
}
}

namespace {
  // compute inertia tensor for particles ds with origin center
IMP_Eigen::Matrix3d compute_I(Model *model,
                              const ParticleIndexes &ds,
                              const algebra::Vector3D &center,
                              const IMP::algebra::Rotation3D &rot) {
  IMP_Eigen::Matrix3d I = IMP_Eigen::Matrix3d::Zero();
  for (unsigned int pi = 0; pi < ds.size(); ++pi) {
    XYZ cm(model, ds[pi]);
    double m = 1;
    double r = 0;
    algebra::Vector3D cv = rot.get_rotated(cm.get_coordinates() - center);

    IMP_Eigen::Matrix3d Is;
    for (unsigned int i = 0; i < 3; ++i) {
      for (unsigned int j = 0; j < 3; ++j) {
        Is(i, j) = -m * cv[i] * cv[j];
        if (i == j) {
          Is(i, j) += m * cv.get_squared_magnitude() + .4 * m * square(r);
        }
      }
    }
    I += Is;
  }
  return I;
}

bool is_rotation_valid(IMP_Eigen::Matrix3d rm) {
  for (unsigned i = 0; i < 3; ++i) {
    for (unsigned j = i; j < 3; ++j) {
      double expected_dot = (i == j) ? 1. : 0.;
      if (std::abs(rm.col(i).dot(rm.col(j)) - expected_dot) >= .1) {
        return false;
      }
      if (std::abs(rm.row(i).dot(rm.row(j)) - expected_dot) >= .1) {
        return false;
      }
    }
  }
  return true;
}
}

void RigidBody::on_change() {
  // Compute maximal $l_inf$ sphere distance from reference frame
  double md = 0;
  {
    // point members
    const ParticleIndexes &members = get_member_particle_indexes();
    for (unsigned int i = 0; i < members.size(); ++i) {
      double cd = (get_coordinates() - XYZ(get_model(), members[i])
                                           .get_coordinates()).get_magnitude();
      if (get_model()->get_has_attribute(XYZR::get_radius_key(), members[i])) {
        cd += get_model()->get_attribute(XYZR::get_radius_key(), members[i]);
      }
      md = std::max(cd, md);
    }
  }
  {
    // rigid body members
    const ParticleIndexes &members = get_body_member_particle_indexes();
    for (unsigned int i = 0; i < members.size(); ++i) {
      double cd = (get_coordinates() - XYZ(get_model(), members[i])
                                           .get_coordinates()).get_magnitude();
      if (get_model()->get_has_attribute(XYZR::get_radius_key(), members[i])) {
        cd += get_model()->get_attribute(XYZR::get_radius_key(), members[i]);
      }
      md = std::max(cd, md);
    }
  }
  // sets radius to point with maximal distance from reference frame
  if (get_particle()->has_attribute(XYZR::get_radius_key())) {
    get_particle()->set_value(XYZR::get_radius_key(), md);
  } else {
    get_particle()->add_attribute(XYZR::get_radius_key(), md);
  }
  // clear caches
  for (unsigned int i = 0; i < cache_keys.size(); ++i) {
    if (get_particle()->has_attribute(cache_keys[i])) {
      get_particle()->remove_attribute(cache_keys[i]);
    }
  }
  get_particle()->get_model()->clear_caches();
}

void RigidBody::teardown_constraints(Particle *p) {
  IMP_FUNCTION_LOG;
  IMP_LOG_TERSE("Tearing down rigid body: " << p->get_name() << std::endl);
  if (p->has_attribute(get_rb_score_state_0_key())) {
    IMP_LOG_TERSE("Remove update coordinates" << std::endl);
    Object *o0 = p->get_value(get_rb_score_state_0_key());
    p->get_model()->remove_score_state(dynamic_cast<ScoreState *>(o0));
    p->remove_attribute(get_rb_score_state_0_key());
  }
  ModelKey mk = get_rb_list_key();
  if (p->get_model()->get_has_data(mk)) {
    IMP_LOG_TERSE("Remove from normalize list" << std::endl);
    Object *o = p->get_model()->get_data(mk);
    IMP::internal::StaticListContainer<SingletonContainer> *list =
        dynamic_cast<IMP::internal::StaticListContainer<
            SingletonContainer> *>(o);
    list->remove(IMP::internal::get_index(p));
#if IMP_HAS_CHECKS >= IMP_INTERNAL
    IMP_CONTAINER_FOREACH(SingletonContainer, list, {
      IMP_CHECK_VARIABLE(_1);
      IMP_INTERNAL_CHECK(_1 != p->get_index(), "Index was not removed");
    });
#endif
  }
}

void RigidBody::do_setup_particle(Model *m, ParticleIndex pi,
                                  ParticleIndexesAdaptor members) {
  IMP_FUNCTION_LOG;
  // IMP_LOG_VERBOSE( "Creating rigid body from other rigid bodies"<<std::endl);
  IMP_USAGE_CHECK(members.size() > 0, "Must provide members");
  algebra::ReferenceFrame3D rf = get_initial_reference_frame(m, members);
  RigidBody rb = setup_particle(m, pi, rf);
  for (unsigned int i = 0; i < members.size(); ++i) {
    rb.add_member(members[i]);
    // IMP_LOG_VERBOSE( " " << cm << " | " << std::endl);
  }
  rb.on_change();
}

void RigidBody::do_setup_particle(Model *m, ParticleIndex pi,
                                  const algebra::ReferenceFrame3D &rf) {
  IMP_FUNCTION_LOG;
  Particle *p = m->get_particle(pi);
  internal::add_required_attributes_for_body(m, pi);
  RigidBody d(p);
  d.set_reference_frame(rf);
  // Include particle in model list of rigid bodies, over which
  // a container optimizer state normalizes the rotation before score calc.
  ModelKey mk = get_rb_list_key();
  if (d.get_model()->get_has_data(mk)) {
    // IMP_LOG_TERSE( "Adding particle to list of rigid bodies" << std::endl);
    Object *o = d.get_model()->get_data(mk);
    IMP::internal::StaticListContainer<SingletonContainer> *list =
        dynamic_cast<IMP::internal::StaticListContainer<
            SingletonContainer> *>(o);
    list->add(IMP::internal::get_index(p));
  } else {
    // IMP_LOG_TERSE( "Creating new list of rigid bodies" << std::endl);
    IMP_NEW(IMP::internal::StaticListContainer<SingletonContainer>,
            list, (d.get_model(), "rigid bodies list"));
    list->set(ParticleIndexes(1, p->get_index()));
    IMP_NEW(NormalizeRotation, nr, ());
    IMP_NEW(NullSDM, null, ());
    Pointer<Constraint> c1 = IMP::internal::create_container_constraint(
        nr.get(), null.get(), list.get(), "normalize rigid bodies");
    d.get_model()->add_score_state(c1);
    d.get_model()->add_data(mk, list);
  }
}

void RigidBody::teardown_particle(RigidBody rb) {
  IMP_FUNCTION_LOG;
  // clear caches
  rb.on_change();
  {
    const ParticleIndexes &members = rb.get_member_particle_indexes();
    IMP_FOREACH(ParticleIndex pi, members) {
      internal::remove_required_attributes_for_member(rb.get_model(), pi);
    }
  }
  {
    const ParticleIndexes &members =
        rb.get_body_member_particle_indexes();
    IMP_FOREACH(ParticleIndex pi, members) {
      internal::remove_required_attributes_for_body_member(rb.get_model(), pi);
    }
  }
  teardown_constraints(rb.get_particle());
  internal::remove_required_attributes_for_body(rb.get_model(), rb);
}

void RigidBody::set_reference_frame_from_members(
    const ParticleIndexes &rms) {
  algebra::Vector3Ds local;
  algebra::Vector3Ds global;
  IMP_USAGE_CHECK(rms.size() >= 3,
                  "Can't initialize a rigid body reference frame "
                      << "with < 3 particles.");
  Model *m = get_model();
  IMP_FOREACH(ParticleIndex pi, rms) {
    local.push_back(RigidMember(m, pi).get_internal_coordinates());
    global.push_back(RigidMember(m, pi).get_coordinates());
  }
  algebra::Transformation3D t3 =
      algebra::get_transformation_aligning_first_to_second(local, global);
  set_reference_frame_lazy(algebra::ReferenceFrame3D(t3));
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    IMP_FOREACH(ParticleIndex pi, rms) {
      algebra::Vector3D local =
          RigidBodyMember(m, pi).get_internal_coordinates();
      algebra::Vector3D back = t3.get_transformed(local);
      algebra::Vector3D global = RigidMember(m, pi).get_coordinates();
      IMP_INTERNAL_CHECK(get_distance(back, global) < 1,
                         "Coordinates don't match: read "
                             << global << " had local " << local << " but got "
                             << back << " with transform " << t3 << " for "
                             << m->get_particle_name(pi));
    }
  }
  // later patch members to make coordinates exact.
  // must reset collision detection tree when we do that
}

void RigidBody::update_members() {
  algebra::Transformation3D tr = get_reference_frame().get_transformation_to();
  {
    const ParticleIndexes &members = get_member_particle_indexes();
    Model *m = get_model();
    for (unsigned int i = 0; i < members.size(); ++i) {
      XYZ rm(get_model(), members[i]);
      algebra::Vector3D v = m->get_internal_coordinates(members[i]);
      rm.set_coordinates(tr.get_transformed(v));
    }
  }
  {
    const ParticleIndexes &members = get_body_member_particle_indexes();
    for (unsigned int i = 0; i < members.size(); ++i) {
      RigidBody rb(get_model(), members[i]);
      algebra::Transformation3D itr =
          RigidBodyMember(get_model(),
                          members[i]).get_internal_transformation();
      rb.set_reference_frame_lazy(algebra::ReferenceFrame3D(tr * itr));
    }
  }
}

RigidMembers RigidBody::get_rigid_members() const {
  RigidMembers ret;
  {
    ParticleIndexes pis = get_member_particle_indexes();
    for (unsigned int i = 0; i < pis.size(); ++i) {
      if (RigidMember::get_is_setup(get_model()->get_particle(pis[i]))) {
        ret.push_back(RigidMember(get_model(), pis[i]));
      }
    }
  }
  {
    ParticleIndexes pis = get_body_member_particle_indexes();
    for (unsigned int i = 0; i < pis.size(); ++i) {
      if (RigidMember::get_is_setup(get_model()->get_particle(pis[i]))) {
        ret.push_back(RigidMember(get_model(), pis[i]));
      }
    }
  }
  return ret;
}

void RigidBody::set_is_rigid_member(ParticleIndex pi, bool tf) {
  get_model()->set_attribute(internal::rigid_body_data().is_rigid_key_, pi,
                             tf ? 1 : 0);
  on_change();
}

void RigidBody::setup_score_states() {
  if (!get_model()->get_has_attribute(get_rb_score_state_0_key(),
                                      get_particle_index())) {
    IMP_NEW(UpdateRigidBodyMembers, urbm, ());
    IMP_NEW(AccumulateRigidBodyDerivatives, arbd, ());
    Pointer<Constraint> c0 = IMP::internal::create_tuple_constraint(
        urbm.get(), arbd.get(), get_particle(),
        get_particle()->get_name() + " rigid body positions");
    get_model()->add_score_state(c0);
    get_model()->add_attribute(get_rb_score_state_0_key(), get_particle_index(),
                               c0);
  }
}

void RigidBody::add_rigid_body_member(ParticleIndex pi) {
  IMP_FUNCTION_LOG;
  algebra::ReferenceFrame3D r = get_reference_frame();
  /*IMP_LOG_TERSE( "Adding rigid body " << p->get_name()
    << " as member." << std::endl);*/
  RigidBody d(get_model(), pi);
  internal::add_required_attributes_for_body_member(get_model(), d,
                                                    get_particle_index());
  RigidMember rm(d);
  // add / set in list of rigid body members
  if (get_model()->get_has_attribute(internal::rigid_body_data().body_members_,
                                     get_particle_index())) {
    ParticleIndexes members = get_model()->get_attribute(
        internal::rigid_body_data().body_members_, get_particle_index());
    members.push_back(pi);
    get_model()->set_attribute(internal::rigid_body_data().body_members_,
                               get_particle_index(), members);
  } else {
    get_model()->add_attribute(internal::rigid_body_data().body_members_,
                               get_particle_index(),
                               ParticleIndexes(1, pi));
  }
  algebra::ReferenceFrame3D pr = d.get_reference_frame();
  algebra::Transformation3D tr =
      r.get_transformation_from() * pr.get_transformation_to();
  rm.set_internal_transformation(tr);
}

void RigidBody::add_point_member(ParticleIndex pi) {
  algebra::ReferenceFrame3D r = get_reference_frame();
  internal::add_required_attributes_for_member(get_model(), pi,
                                               get_particle_index());
  RigidMember cm(get_model(), pi);
  if (get_model()->get_has_attribute(internal::rigid_body_data().members_,
                                     get_particle_index())) {
    ParticleIndexes members = get_model()->get_attribute(
        internal::rigid_body_data().members_, get_particle_index());
    members.push_back(pi);
    get_model()->set_attribute(internal::rigid_body_data().members_,
                               get_particle_index(), members);
  } else {
    get_model()->add_attribute(internal::rigid_body_data().members_,
                               get_particle_index(),
                               ParticleIndexes(1, pi));
  }
  algebra::Vector3D lc =
      r.get_local_coordinates(XYZ(get_model(), pi).get_coordinates());
  cm.set_internal_coordinates(lc);
  IMP_USAGE_CHECK((cm.get_internal_coordinates() - lc).get_magnitude() < .1,
                  "Bad setting of coordinates.");
}

void RigidBody::add_member(ParticleIndexAdaptor pi) {
  IMP_FUNCTION_LOG;
  algebra::ReferenceFrame3D r = get_reference_frame();
  if (RigidBody::get_is_setup(get_model(), pi)) {
    add_rigid_body_member(pi);
  } else {
    add_point_member(pi);
  }
  setup_score_states();
  on_change();
}

void RigidBody::add_non_rigid_member(ParticleIndexAdaptor pi) {
  IMP_FUNCTION_LOG;
  add_member(pi);
  set_is_rigid_member(pi, false);
}

algebra::VectorD<4> RigidBody::get_rotational_derivatives() const {
  algebra::VectorD<4> v(get_particle()->get_derivative(
                            internal::rigid_body_data().quaternion_[0]),
                        get_particle()->get_derivative(
                            internal::rigid_body_data().quaternion_[1]),
                        get_particle()->get_derivative(
                            internal::rigid_body_data().quaternion_[2]),
                        get_particle()->get_derivative(
                            internal::rigid_body_data().quaternion_[3]));
  return v;
}

bool RigidBody::get_coordinates_are_optimized() const {
  for (unsigned int i = 0; i < 4; ++i) {
    if (!get_particle()->get_is_optimized(
             internal::rigid_body_data().quaternion_[i]))
      return false;
  }
  return XYZ::get_coordinates_are_optimized();
}

void RigidBody::set_coordinates_are_optimized(bool tf) {
  const bool body = tf;
  const bool member = false;
  for (unsigned int i = 0; i < 4; ++i) {
    get_particle()->set_is_optimized(internal::rigid_body_data().quaternion_[i],
                                     body);
  }
  XYZ::set_coordinates_are_optimized(body);
  for (unsigned int i = 0; i < get_number_of_members(); ++i) {
    get_member(i).set_coordinates_are_optimized(member);
  }
}

RigidMember RigidBody::get_member(unsigned int i) const {
  IMP_USAGE_CHECK(i < get_number_of_members(),
                  "Out of range member requested: " << i << " of "
                                                    << get_number_of_members());
  unsigned int sz = get_member_particle_indexes().size();
  if (i < sz) {
    return RigidMember(get_model(), get_member_particle_indexes()[i]);
  } else {
    return RigidMember(get_model(), get_body_member_particle_indexes()[i - sz]);
  }
}

algebra::Vector3D RigidBody::get_coordinates(RigidMember p) const {
  algebra::Vector3D lp = p.get_internal_coordinates();
  return get_reference_frame().get_global_coordinates(lp);
}

void RigidBody::set_reference_frame(const IMP::algebra::ReferenceFrame3D &tr) {
  set_reference_frame_lazy(tr);
  update_members();
}

RigidBody::~RigidBody() {}
RigidBodyMember::~RigidBodyMember() {}
RigidMember::~RigidMember() {}
NonRigidMember::~NonRigidMember() {}

void RigidBody::show(std::ostream &out) const {
  if(Decorator::get_is_valid()){
    out << "Rigid body " << get_reference_frame();
  } else {
    out << "Invalid rigid body" ;
  }
}

void RigidBodyMember::show(std::ostream &out) const {
  if(Decorator::get_is_valid()){
    out << "Member of " << get_rigid_body()->get_name() << " at "
        << get_internal_coordinates();
  } else {
    out << "Invalid rigid body member";
  }
}

void RigidMember::show(std::ostream &out) const {
  RigidBodyMember::show(out);
  out << " (rigid)";
}

RigidBody RigidBodyMember::get_rigid_body() const {
  return RigidBody(
      get_particle()->get_value(internal::rigid_body_data().body_));
}

void NonRigidMember::show(std::ostream &out) const {
  RigidBodyMember::show(out);
  out << " (non-rigid)";
}

bool RigidMembersRefiner::get_can_refine(Particle *p) const {
  return RigidBody::get_is_setup(p);
}
const ParticlesTemp RigidMembersRefiner::get_refined(
    Particle *p) const {
  return RigidBody(p).get_rigid_members();
}

ModelObjectsTemp RigidMembersRefiner::do_get_inputs(
    Model *m, const ParticleIndexes &pis) const {
  return IMP::get_particles(m, pis);
}

namespace internal {
RigidMembersRefiner *get_rigid_members_refiner() {
  static IMP::PointerMember<RigidMembersRefiner> pt =
      new RigidMembersRefiner("The rigid members refiner");
  return pt;
}
}

namespace {
bool check_rigid_body(Model *m, ParticleIndex pi) {
  algebra::Vector4D v(
      m->get_attribute(internal::rigid_body_data().quaternion_[0], pi),
      m->get_attribute(internal::rigid_body_data().quaternion_[1], pi),
      m->get_attribute(internal::rigid_body_data().quaternion_[2], pi),
      m->get_attribute(internal::rigid_body_data().quaternion_[3], pi));
  if (std::abs(v.get_magnitude() - 1) > .1) {
    IMP_THROW("Bad quaternion in rigid body: " << v, ValueException);
  }
  return true;
}
}

IMP_CHECK_DECORATOR(RigidBody, check_rigid_body);

// returns reference frame with center of mass of ps and the
// diagonalized inertia tensor of ps
algebra::ReferenceFrame3D get_initial_reference_frame(
    Model *m, const ParticleIndexes &ps) {
  if (ps.size() == 1) {
    if (RigidBody::get_is_setup(m, ps[0])) {
      return RigidBody(m, ps[0]).get_reference_frame();
    } else {
      algebra::Transformation3D tr(algebra::get_identity_rotation_3d(),
                                   XYZ(m, ps[0]).get_coordinates());
      return algebra::ReferenceFrame3D(tr);
    }
  }
  IMP_USAGE_CHECK(!ps.empty(),
                  "There must be particles to make a reference frame");

  // compute center of mass
  algebra::Vector3D v(0, 0, 0);
  double mass = 0;
  for (unsigned int i = 0; i < ps.size(); ++i) {
    double cmass = 1.0;
    if (XYZR::get_is_setup(m, ps[i]) && XYZR(m, ps[i]).get_radius() > 0) {
      cmass = std::pow(XYZR(m, ps[i]).get_radius(), 3);
    }
    v += XYZ(m, ps[i]).get_coordinates() * cmass /*cm.get_mass()*/;
    mass += cmass /*cm.get_mass()*/;
  }
  IMP_USAGE_CHECK(mass > 0, "Zero mass when computing axis.");
  v /= mass;
  // IMP_LOG_VERBOSE( "Center of mass is " << v << std::endl);
  // for a sphere 2/5 m r^2 (diagonal)
  // parallel axis theorem
  // I'ij= Iij+M(v^2delta_ij-vi*vj)
  // compute I
  IMP_Eigen::Matrix3d I =
      compute_I(m, ps, v, IMP::algebra::get_identity_rotation_3d());
  // IMP_LOG_VERBOSE( "Initial I is " << I << std::endl);
  // diagonalize it
  IMP_Eigen::EigenSolver<IMP_Eigen::Matrix3d> eig(I);
  IMP_Eigen::Matrix3d rm = eig.eigenvectors().real();
  if (rm.determinant() < 0) {
    rm.array() *= -1.0;
  }
  IMP::algebra::Rotation3D rot;
  if (is_rotation_valid(rm)) {
    // use the R as the initial orientation
    rot = IMP::algebra::get_rotation_from_matrix(rm);
  } else {
    // otherwise, use the identity matrix (this can happen when I is close
    // to - but not exactly - the identity matrix, which is possible for
    // very small numbers of particles)
    rot = IMP::algebra::get_identity_rotation_3d();
  }
  // IMP_LOG_VERBOSE( "Initial rotation is " << rot << std::endl);
  return algebra::ReferenceFrame3D(algebra::Transformation3D(rot, v));
}

ParticlesTemp create_rigid_bodies(Model *m, unsigned int n,
                                  bool no_members) {
  ParticlesTemp ret(n);
  for (unsigned int i = 0; i < n; ++i) {
    ParticleIndex pi = m->add_particle("RB%1%");
    ret[i] = m->get_particle(pi);
    RigidBody::setup_particle(m, pi, algebra::ReferenceFrame3D());
  }
  IMP_NEW(IMP::internal::StaticListContainer<SingletonContainer>,
          list, (m, "rigid body list"));
  list->set(IMP::internal::get_index(ret));
  if (!no_members) {
    IMP_NEW(UpdateRigidBodyMembers, urbm, ());
    IMP_NEW(AccumulateRigidBodyDerivatives, arbd, ());
    Pointer<Constraint> c0 = IMP::internal::create_container_constraint(
        urbm.get(), arbd.get(), list.get(), "rigid body positions %1%");
    m->add_score_state(c0);
    for (unsigned int i = 0; i < ret.size(); ++i) {
      m->add_attribute(get_rb_score_state_0_key(), ret[i]->get_index(), c0);
    }
  }
  return ret;
}

void show_rigid_body_hierarchy(RigidBody rb, TextOutput out) {
  Model *m = rb.get_model();
  Vector<boost::tuple<std::string, std::string, ParticleIndex> >
      queue;
  queue.push_back(
      boost::make_tuple(std::string(), std::string(), rb.get_particle_index()));
  do {
    ParticleIndex pi = queue.back().get<2>();
    out.get_stream() << queue.back().get<0>();
    std::string prefix1 = queue.back().get<1>();
    queue.pop_back();
    if (core::RigidBody::get_is_setup(m, pi)) {
      out.get_stream() << " + " << m->get_particle_name(pi) << std::endl;
      core::RigidBody rb(m, pi);
      IMP_FOREACH(ParticleIndex ch,
                  rb.get_member_particle_indexes() +
                      rb.get_body_member_particle_indexes()) {
        queue.push_back(boost::make_tuple(prefix1 + " ", prefix1 + " ", ch));
      }
    } else {
      out.get_stream() << " - " << m->get_particle_name(pi) << std::endl;
    }
  } while (!queue.empty());
}

ParticleIndex get_root_rigid_body(RigidMember m) {
  core::RigidBody body = m.get_rigid_body();
  while (RigidMember::get_is_setup(body)) {
    body = RigidBodyMember(body).get_rigid_body();
  }
  return body.get_particle_index();
}

IMPCORE_END_NAMESPACE
