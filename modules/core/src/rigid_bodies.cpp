/**
 *  \file rigid_bodies.cpp
 *  \brief Support for rigid bodies.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/core/rigid_bodies.h"
#include "IMP/core/SingletonConstraint.h"
#include <IMP/internal/ContainerConstraint.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/internal/tnt_array2d.h>
#include <IMP/algebra/internal/tnt_array2d_utils.h>
#include <IMP/algebra/internal/jama_eig.h>
#include <IMP/algebra/geometric_alignment.h>
#include "IMP/generic.h"
#include <IMP/SingletonContainer.h>
#include <IMP/core/FixedRefiner.h>
#include <IMP/core/internal/rigid_body_tree.h>
#include <IMP/internal/InternalListSingletonContainer.h>
#include <IMP/algebra/geometric_alignment.h>


IMPCORE_BEGIN_INTERNAL_NAMESPACE


const RigidBodyData &rigid_body_data() {
  static const RigidBodyData rbd;
  return rbd;
}
IMPCORE_END_INTERNAL_NAMESPACE
IMPCORE_BEGIN_NAMESPACE
void
RigidBody::normalize_rotation() {
  double &q0=get_model()->access_attribute(internal::rigid_body_data()
                                           .quaternion_[0],
                                           get_particle_index());
  double &q1=get_model()->access_attribute(internal::rigid_body_data()
                                           .quaternion_[1],
                                           get_particle_index());
  double &q2=get_model()->access_attribute(internal::rigid_body_data()
                                           .quaternion_[2],
                                           get_particle_index());
  double &q3=get_model()->access_attribute(internal::rigid_body_data()
                                           .quaternion_[3],
                                           get_particle_index());
  algebra::VectorD<4> v(q0, q1, q2, q3);
  //IMP_LOG_TERSE( "Rotation was " << v << std::endl);
  double sm= v.get_squared_magnitude();
  if (sm < .001) {
    v= algebra::VectorD<4>(1,0,0,0);
    //IMP_LOG_TERSE( "Rotation is " << v << std::endl);
    q0=1;
    q1=0;
    q2=0;
    q3=0;
  } else if (std::abs(sm-1.0) > .01){
    v= v.get_unit_vector();
    //IMP_LOG_TERSE( "Rotation is " << v << std::endl);
    q0=v[0];
    q1=v[1];
    q2=v[2];
    q3=v[3];
  }

  // evil hack
  get_model()->set_attribute(internal::rigid_body_data().torque_[0],
                             get_particle_index(), 0);
  get_model()->set_attribute(internal::rigid_body_data().torque_[1],
                             get_particle_index(), 0);
  get_model()->set_attribute(internal::rigid_body_data().torque_[2],
                             get_particle_index(), 0);
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
  class AccumulateRigidBodyDerivatives:
    public SingletonDerivativeModifier {
  public:
    AccumulateRigidBodyDerivatives(std::string name
                                   ="AccumulateRigidBodyDerivatives%1%"):
        SingletonDerivativeModifier(name){}
    IMP_INDEX_SINGLETON_DERIVATIVE_MODIFIER(AccumulateRigidBodyDerivatives);
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
  class UpdateRigidBodyMembers: public SingletonModifier {
  public:
    UpdateRigidBodyMembers(std::string name
                           = "UpdateRigidBodyMembers%1%"):
        SingletonModifier(name) {}
    IMP_INDEX_SINGLETON_MODIFIER(UpdateRigidBodyMembers);
  };


 /** \brief Fix the normalization of the rotation term. */
  class NormalizeRotation: public SingletonModifier {
  public:
    NormalizeRotation(std::string name
                      = "NormalizeRotation%1%"):
        SingletonModifier(name){}
    IMP_INDEX_SINGLETON_MODIFIER(NormalizeRotation);
  };


 /** \brief Fix the normalization of the rotation term. */
  class NullSDM: public SingletonDerivativeModifier {
  public:
    NullSDM(std::string name="NullModifier%1%"):
        SingletonDerivativeModifier(name){}
    IMP_INDEX_SINGLETON_DERIVATIVE_MODIFIER(NullSDM);
  };



  void AccumulateRigidBodyDerivatives::apply_index(Model *m,
                                                   ParticleIndex pi) const {
    IMP_OBJECT_LOG;
    DerivativeAccumulator da;
    RigidBody rb(m, pi);
#if IMP_HAS_CHECKS >= IMP_INTERNAL
    algebra::Vector4D oldderiv;
    algebra::Vector3D oldcartesian= rb.get_derivatives();
    for (unsigned int j=0; j< 4; ++j) {
      oldderiv[j]=rb.get_particle()->get_derivative(internal::rigid_body_data()
                                                    .quaternion_[j]);
    }
#endif
    algebra::Rotation3D rot= rb.get_reference_frame().get_transformation_from()\
      .get_rotation();
    algebra::Rotation3D roti= rb.get_reference_frame().get_transformation_to()
      .get_rotation();
    const ParticleIndexes &rbis= rb.get_member_particle_indexes();
    for (unsigned int i=0; i< rbis.size(); ++i) {
      RigidMember d(rb.get_model(), rbis[i]);
      const algebra::Vector3D &deriv=d.get_derivatives();
      if (deriv.get_squared_magnitude() > 0) {
        algebra::Vector3D dv= rot*deriv;
        rb.add_to_derivatives(dv, deriv, d.get_internal_coordinates(),
                              roti, da);
      }
    }
    const ParticleIndexes &rbbis= rb.get_body_member_particle_indexes();
    for (unsigned int i=0; i< rbbis.size(); ++i) {
      RigidMember d(rb.get_model(), rbbis[i]);
      const algebra::Vector3D &deriv=d.get_derivatives();
      if (deriv.get_squared_magnitude() > 0) {
        algebra::Vector3D dv= rot*deriv;
        rb.add_to_derivatives(dv, deriv,
                              d.get_internal_coordinates(), roti, da);
      }
    }
    // ignoring torques on rigid members
    IMP_LOG_TERSE( "Rigid body derivative is "
            << m->get_particle(pi)
            ->get_derivative(internal::rigid_body_data().quaternion_[0])
            << " "
            << m->get_particle(pi)
            ->get_derivative(internal::rigid_body_data().quaternion_[1])
            << " "
            << m->get_particle(pi)
            ->get_derivative(internal::rigid_body_data().quaternion_[2])
            << " "
            << m->get_particle(pi)
            ->get_derivative(internal::rigid_body_data().quaternion_[3])
            << " and ");

    IMP_LOG_TERSE( "Translation deriv is "
            << static_cast<XYZ>(rb).get_derivatives()
            << "" << std::endl);
    IMP_IF_CHECK(USAGE_AND_INTERNAL) {
      algebra::Rotation3D rot= rb.get_reference_frame()
        .get_transformation_to().get_rotation();
      //IMP_LOG_TERSE( "Accumulating rigid body derivatives" << std::endl);
      algebra::Vector3D v(0,0,0);
      algebra::VectorD<4> q(0,0,0,0);
      for (unsigned int i=0; i< rb.get_number_of_members(); ++i) {
        RigidMember d= rb.get_member(i);
        algebra::Vector3D dv= d.get_derivatives();
        v+=dv;
        //IMP_LOG_TERSE( "Adding " << dv << " to derivative" << std::endl);
        for (unsigned int j=0; j< 4; ++j) {
          algebra::Vector3D v
            = rot.get_derivative(d.get_internal_coordinates(),
                                 j);
          /*IMP_LOG_VERBOSE( "Adding " << dv*v << " to quaternion deriv " << j
            << std::endl);*/
          q[j]+= dv*v;
        }
      }
      for (unsigned int j=0; j< 4; ++j) {
#if IMP_HAS_CHECKS >= IMP_INTERNAL
        double d= rb.get_particle()->get_derivative(internal::rigid_body_data()
                                                    .quaternion_[j])
          - oldderiv[j];
#endif
        IMP_INTERNAL_CHECK(std::abs(d-q[j])< .05*std::abs(d+q[j])+.05,
                           "Derivatives do not match "
                           << oldderiv << ": "
                           << rb.get_particle()
                           ->get_derivative(internal::rigid_body_data()
                                            .quaternion_[0])
                           << " " << rb.get_particle()
                           ->get_derivative(internal::rigid_body_data()
                                            .quaternion_[1])
                           << " " << rb.get_particle()
                           ->get_derivative(internal::rigid_body_data()
                                            .quaternion_[1])
                           << " " << rb.get_particle()
                           ->get_derivative(internal::rigid_body_data()
                                            .quaternion_[2])
                           << ": " << q);
      }
#if IMP_HAS_CHECKS >= IMP_INTERNAL
      algebra::Vector3D deltacartesian= rb.get_derivatives()-oldcartesian;
#endif
      IMP_INTERNAL_CHECK((deltacartesian-v).get_magnitude()
                         < .01*(v+deltacartesian).get_magnitude()+.1,
                         "Cartesian derivatives don't match : "
                         << deltacartesian << " vs " << v);
    }
  }


  IMP_SINGLETON_MODIFIER_FROM_REFINED(AccumulateRigidBodyDerivatives,
                                      internal::get_rigid_members_refiner());


  void UpdateRigidBodyMembers::apply_index(Model *m,
                                           ParticleIndex pi) const {
    RigidBody rb(m, pi);
    rb.update_members();
  }
  ModelObjectsTemp
  UpdateRigidBodyMembers::do_get_inputs(Model *m,
                                        const ParticleIndexes &pis) const {
    ModelObjectsTemp ret;
    ret+= IMP::get_particles(m, pis);
    return ret;
  }
  ModelObjectsTemp
  UpdateRigidBodyMembers::do_get_outputs(Model *m,
                                        const ParticleIndexes &pis) const {
    ModelObjectsTemp ret;
    for (unsigned int i=0; i< pis.size(); ++i) {
      RigidBody rb(m, pis[i]);
      ret += rb.get_members();
      ret += IMP::get_particles(m, rb.get_non_rigid_member_particle_indexes());
    }
    return ret;
  }
  void UpdateRigidBodyMembers::do_show(std::ostream &) const {
  }


inline void NormalizeRotation::apply_index(Model *m, ParticleIndex p) const {
  double &q0=m->access_attribute(internal::rigid_body_data()
                                 .quaternion_[0],
                                 p);
  double &q1=m->access_attribute(internal::rigid_body_data()
                                .quaternion_[1],
                                p);
  double &q2=m->access_attribute(internal::rigid_body_data()
                                 .quaternion_[2],
                                 p);
  double &q3=m->access_attribute(internal::rigid_body_data()
                                 .quaternion_[3],
                                 p);
  algebra::VectorD<4> v(q0, q1, q2, q3);
  //IMP_LOG_TERSE( "Rotation was " << v << std::endl);
  double sm= v.get_squared_magnitude();
  if (sm < .001) {
    v= algebra::VectorD<4>(1,0,0,0);
    //IMP_LOG_TERSE( "Rotation is " << v << std::endl);
    q0=1;
    q1=0;
    q2=0;
    q3=0;
  } else if (std::abs(sm-1.0) > .01){
    v= v.get_unit_vector();
    //IMP_LOG_TERSE( "Rotation is " << v << std::endl);
    q0=v[0];
    q1=v[1];
    q2=v[2];
    q3=v[3];
  }

  // evil hack
  m->set_attribute(internal::rigid_body_data().torque_[0],
                   p, 0);
  m->set_attribute(internal::rigid_body_data().torque_[1],
                   p, 0);
  m->set_attribute(internal::rigid_body_data().torque_[2],
                  p, 0);
}
  ModelObjectsTemp NormalizeRotation::do_get_inputs(Model*m,
                                                    const ParticleIndexes &pis)
    const {
    return IMP::get_particles(m, pis);
  }
  ModelObjectsTemp NormalizeRotation::do_get_outputs(Model*m,
                                                    const ParticleIndexes &pis)
    const {
    return IMP::get_particles(m, pis);
  }
  void NormalizeRotation::do_show(std::ostream &) const {
  }

  inline void NullSDM::apply_index(Model *, ParticleIndex) const {
  }
  ModelObjectsTemp NullSDM::do_get_inputs(Model*,
                                                    const ParticleIndexes &)
    const {
    return ModelObjectsTemp();
  }
  ModelObjectsTemp NullSDM::do_get_outputs(Model*,
                                                    const ParticleIndexes &)
    const {
    return ModelObjectsTemp();
  }
  void NullSDM::do_show(std::ostream &) const {
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

typedef IMP::algebra::internal::TNT::Array2D<double> Matrix;

namespace {
Matrix compute_I(Model *model,
                 const ParticleIndexes &ds,
                 const algebra::Vector3D &center,
                 const IMP::algebra::Rotation3D &rot) {
  Matrix I(3,3, 0.0);
  for (unsigned int i=0; i< ds.size(); ++i) {
    XYZ cm(model, ds[i]);
    double m=1;
    double r=0;
    algebra::Vector3D cv=rot.get_rotated(cm.get_coordinates()-center);

    Matrix Is(3,3, 0.0);
    for (unsigned int i=0; i<3; ++i) {
      for (unsigned int j=0; j<3; ++j) {
        Is[i][j]= -m*cv[i]*cv[j];
        if (i==j) {
          Is[i][j]+= m*cv.get_squared_magnitude() + .4*m*square(r);
        }
      }
    }
    I+= Is;
  }
  return I;
}
}

void RigidBody::on_change() {
   double md=0;
   {
     const ParticleIndexes& members= get_member_particle_indexes();
     for (unsigned int i=0; i< members.size(); ++i) {
       double cd= (get_coordinates()
                   -XYZ(get_model(),
                        members[i]).get_coordinates()).get_magnitude();
       if (get_model()->get_has_attribute(XYZR::get_radius_key(),
                                          members[i])) {
         cd+= get_model()->get_attribute(XYZR::get_radius_key(),
                                         members[i]);
       }
       md=std::max(cd, md);
     }
   }
   {
     const ParticleIndexes& members= get_body_member_particle_indexes();
     for (unsigned int i=0; i< members.size(); ++i) {
       double cd= (get_coordinates()
                   -XYZ(get_model(),
                        members[i]).get_coordinates()).get_magnitude();
       if (get_model()->get_has_attribute(XYZR::get_radius_key(), members[i])) {
         cd+= get_model()->get_attribute(XYZR::get_radius_key(), members[i]);
       }
       md=std::max(cd, md);
     }
   }
   if (get_particle()->has_attribute(XYZR::get_radius_key())) {
     get_particle()->set_value(XYZR::get_radius_key(), md);
   } else {
     get_particle()->add_attribute(XYZR::get_radius_key(), md);
   }
   for (unsigned int i=0; i< cache_keys.size(); ++i) {
     if (get_particle()
         ->has_attribute(cache_keys[i])) {
       get_particle()->remove_attribute(cache_keys[i]);
     }
   }
   get_particle()->get_model()->clear_caches();
}


void RigidBody::teardown_constraints(Particle *p) {
  IMP_FUNCTION_LOG;
  IMP_LOG_TERSE( "Tearing down rigid body: " << p->get_name() << std::endl);
  if (p->has_attribute(get_rb_score_state_0_key())) {
    IMP_LOG_TERSE( "Remove update coordinates" << std::endl);
    Object *o0= p->get_value(get_rb_score_state_0_key());
    p->get_model()->remove_score_state(dynamic_cast<ScoreState*>(o0));
    p->remove_attribute(get_rb_score_state_0_key());
  }
  ModelKey mk= get_rb_list_key();
  if (p->get_model()->get_has_data(mk)) {
    IMP_LOG_TERSE( "Remove from normalize list" << std::endl);
    Object *o= p->get_model()->get_data(mk);
    IMP::internal::InternalListSingletonContainer* list
      = dynamic_cast<IMP::internal::InternalListSingletonContainer*>(o);
    list->remove(IMP::internal::get_index(p));
#if IMP_HAS_CHECKS >= IMP_INTERNAL
      IMP_FOREACH_SINGLETON_INDEX(list, {
          IMP_CHECK_VARIABLE(_1);
          IMP_INTERNAL_CHECK(_1 != p->get_index(),
                             "Index was not removed");
        });
#endif
  }
}


RigidBody RigidBody::setup_particle(Particle *p,
                                    const ParticlesTemp &members) {
  IMP_FUNCTION_LOG;
  //IMP_LOG_VERBOSE( "Creating rigid body from other rigid bodies"<<std::endl);
  IMP_USAGE_CHECK(members.size() > 0, "Must provide members");
  algebra::ReferenceFrame3D rf= get_initial_reference_frame(members);
  RigidBody rb= setup_particle(p, rf);
  for (unsigned int i=0; i< members.size(); ++i) {
    rb.add_member(members[i]);
    //IMP_LOG_VERBOSE( " " << cm << " | " << std::endl);
  }
  rb.on_change();
  return rb;
}


RigidBody RigidBody::setup_particle(Particle *p,
                                    const algebra::ReferenceFrame3D &rf) {
  IMP_FUNCTION_LOG;
  internal::add_required_attributes_for_body(p);
  RigidBody d(p);
  d.set_reference_frame(rf);
  ModelKey mk= get_rb_list_key();
  if (d.get_model()->get_has_data(mk)) {
    //IMP_LOG_TERSE( "Adding particle to list of rigid bodies" << std::endl);
    Object *o= d.get_model()->get_data(mk);
    IMP::internal::InternalListSingletonContainer* list
      = dynamic_cast<IMP::internal::InternalListSingletonContainer*>(o);
    list->add(IMP::internal::get_index(p));
  } else {
    //IMP_LOG_TERSE( "Creating new list of rigid bodies" << std::endl);
    IMP_NEW(IMP::internal::InternalListSingletonContainer, list, (d.get_model(),
                                                         "rigid bodies list"));
    list->set(ParticleIndexes(1,p->get_index()));
    IMP_NEW(NormalizeRotation, nr, ());
    IMP_NEW(NullSDM, null, ());
    Pointer<Constraint> c1
        = IMP::internal::create_container_constraint(nr.get(), null.get(),
                                      list.get(),"normalize rigid bodies");
    d.get_model()->add_score_state(c1);
    d.get_model()->add_data(mk, list);
  }
  return d;
}



void RigidBody::teardown_particle(RigidBody rb) {
  IMP_FUNCTION_LOG;
  // clear caches
  rb.on_change();
  {
    const ParticleIndexes& members= rb.get_member_particle_indexes();
    for (unsigned int i=0; i < members.size(); ++i) {
      RigidMember rm(rb.get_model(), members[i]);
      internal::remove_required_attributes_for_member(rm);
    }
  }
  {
    const ParticleIndexes& members= rb.get_body_member_particle_indexes();
    for (unsigned int i=0; i < members.size(); ++i) {
      RigidMember rm(rb.get_model(), members[i]);
      internal::remove_required_attributes_for_body_member(rm);
    }
  }
  teardown_constraints(rb.get_particle());
  internal::remove_required_attributes_for_body(rb.get_particle());
}
void RigidBody::set_reference_frame_from_members(const ParticleIndexes &rms) {
  algebra::Vector3Ds local(rms.size());
  algebra::Vector3Ds global(rms.size());
  if (rms.size() < 3) {
    return;
  }
  Model *m= get_model();
  for (unsigned int i=0; i< rms.size(); ++i) {
    local[i]= RigidMember(m, rms[i]).get_internal_coordinates();
    global[i]= RigidMember(m, rms[i]).get_coordinates();
  }
  algebra::Transformation3D t3
    = algebra::get_transformation_aligning_first_to_second(local, global);
  set_reference_frame(algebra::ReferenceFrame3D(t3));
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    for (unsigned int i=0; i< rms.size(); ++i) {
      algebra::Vector3D local= RigidMember(m, rms[i])
        .get_internal_coordinates();
      algebra::Vector3D back= t3.get_transformed(local);
      algebra::Vector3D global= RigidMember(m, rms[i]).get_coordinates();
      IMP_INTERNAL_CHECK(get_distance(back, global) < 1,
                         "Coordinates don't match: read " << global
                         << " had local " << local
                         << " but got " << back
                       << " with transform " << t3);
    }
  }
  // later patch members to make coordinates exact.
  // must reset collision detection tree when we do that
}


void RigidBody::update_members() {
  algebra::Transformation3D tr= get_reference_frame().get_transformation_to();
  {
    const ParticleIndexes&members= get_member_particle_indexes();
    for (unsigned int i=0; i< members.size(); ++i) {
      RigidMember rm(get_model(), members[i]);
      rm.set_coordinates(tr.get_transformed(rm.get_internal_coordinates()));
    }
  }
  {
    const ParticleIndexes&members= get_non_rigid_member_particle_indexes();
    for (unsigned int i=0; i< members.size(); ++i) {
      NonRigidMember rm(get_model(), members[i]);
      rm.set_coordinates(tr.get_transformed(rm.get_internal_coordinates()));
    }
  }
  {
    const ParticleIndexes&members= get_body_member_particle_indexes();
    for (unsigned int i=0; i< members.size(); ++i) {
      RigidBody rb(get_model(), members[i]);
      algebra::Transformation3D itr
          =RigidMember(get_model(),
                       members[i]).get_internal_transformation();
      rb.set_reference_frame_lazy(algebra::ReferenceFrame3D(tr
                                                            *itr));
    }
  }
}


RigidMembers
RigidBody::get_members() const {
  RigidMembers ret;
  {
    ParticleIndexes pis= get_member_particle_indexes();
    for (unsigned int i=0; i< pis.size(); ++i) {
      ret.push_back(RigidMember(get_model(), pis[i]));
    }
  }
  {
   ParticleIndexes pis= get_body_member_particle_indexes();
   for (unsigned int i=0; i< pis.size(); ++i) {
     ret.push_back(RigidMember(get_model(), pis[i]));
    }
  }
  return ret;
}


void RigidBody::add_member(Particle *p) {
  IMP_FUNCTION_LOG;
  algebra::ReferenceFrame3D r= get_reference_frame();
  if (RigidBody::particle_is_instance(p)) {
    /*IMP_LOG_TERSE( "Adding rigid body " << p->get_name()
      << " as member." << std::endl);*/
    RigidBody d(p);
    internal::add_required_attributes_for_body_member(d, get_particle());
    RigidMember cm(d);
    if (get_model()
        ->get_has_attribute(internal::rigid_body_data().body_members_,
                            get_particle_index())) {
      ParticleIndexes members
          =get_model()->get_attribute(internal::rigid_body_data().body_members_,
                                      get_particle_index());
      members.push_back(p->get_index());
      get_model()->set_attribute(internal::rigid_body_data().body_members_,
                                 get_particle_index(), members);
    } else {
      get_model()->add_attribute(internal::rigid_body_data().body_members_,
                                 get_particle_index(),
                                 ParticleIndexes(1, p->get_index()));
    }
    /*IMP_LOG_TERSE( "Body members are "
            << get_model()->get_attribute(internal::rigid_body_data()
                                          .body_members_,
                                          get_particle_index()) << std::endl);*/
    // want tr*ltr= btr, so ltr= tr-1*btr
    algebra::ReferenceFrame3D pr=d.get_reference_frame();
    algebra::Transformation3D tr
        =r.get_transformation_from()*pr.get_transformation_to();
    cm.set_internal_transformation(tr);
    /*IMP_LOG_TERSE( "Transformations are " << r << " and " << pr
            << " (" << r.get_transformation_from() << ")"
            << " resulting in " << cm.get_internal_transformation()
            << " from " << tr
            << " with check of "
            << r.get_transformation_to()*cm.get_internal_transformation()
            << std::endl);*/
  } else {
    /*IMP_LOG_TERSE( "Adding XYZ " << p->get_name()
      << " as member." << std::endl);*/
    internal::add_required_attributes_for_member(p, get_particle());
    RigidMember cm(p);
    if (get_model()->get_has_attribute(internal::rigid_body_data().members_,
                                       get_particle_index())) {
      ParticleIndexes members
          =get_model()->get_attribute(internal::rigid_body_data().members_,
                                      get_particle_index());
      members.push_back(p->get_index());
      get_model()->set_attribute(internal::rigid_body_data().members_,
                                 get_particle_index(), members);
    } else {
      get_model()->add_attribute(internal::rigid_body_data().members_,
                                 get_particle_index(),
                                 ParticleIndexes(1, p->get_index()));
    }
    algebra::Vector3D lc=r.get_local_coordinates(XYZ(p).get_coordinates());
    cm.set_internal_coordinates(lc);
    IMP_USAGE_CHECK((cm.get_internal_coordinates()-lc).get_magnitude() < .1,
                    "Bad setting of coordinates.");
  }

  if (!get_model()->get_has_attribute(get_rb_score_state_0_key(),
                                      get_particle_index())) {
    /*IMP_LOG_TERSE( "Setting up constraint for rigid body "
      << get_particle()->get_name() << std::endl);*/
    IMP_NEW(UpdateRigidBodyMembers, urbm,());
    IMP_NEW(AccumulateRigidBodyDerivatives, arbd, ());
    Pointer<Constraint> c0
        = IMP::internal::create_tuple_constraint(urbm.get(), arbd.get(),
                            get_particle(),
                            get_particle()->get_name()
                            +" rigid body positions");
    get_model()->add_score_state(c0);
    get_model()->add_attribute(get_rb_score_state_0_key(),
                               get_particle_index(), c0);
  }
  on_change();
}


void RigidBody::add_non_rigid_member(ParticleIndex pi) {
  IMP_FUNCTION_LOG;
  algebra::ReferenceFrame3D r= get_reference_frame();
  /*IMP_LOG_TERSE( "Adding XYZ " << p->get_name()
    << " as member." << std::endl);*/
  Particle *p = get_model()->get_particle(pi);
  internal::add_required_attributes_for_non_member(p, get_particle());
  NonRigidMember cm(p);
  if (get_model()->get_has_attribute(internal::rigid_body_data().non_members_,
                                     get_particle_index())) {
    ParticleIndexes members
        =get_model()->get_attribute(internal::rigid_body_data().non_members_,
                                    get_particle_index());
    members.push_back(p->get_index());
    get_model()->set_attribute(internal::rigid_body_data().non_members_,
                               get_particle_index(), members);
  } else {
    get_model()->add_attribute(internal::rigid_body_data().non_members_,
                               get_particle_index(),
                               ParticleIndexes(1, p->get_index()));
  }
  // merge with add_member
  algebra::Vector3D lc=r.get_local_coordinates(XYZ(p).get_coordinates());
  cm.set_internal_coordinates(lc);
  IMP_USAGE_CHECK((cm.get_internal_coordinates()-lc).get_magnitude() < .1,
                  "Bad setting of coordinates.");

  if (!get_model()->get_has_attribute(get_rb_score_state_0_key(),
                                      get_particle_index())) {
    /*IMP_LOG_TERSE( "Setting up constraint for rigid body "
      << get_particle()->get_name() << std::endl);*/
    IMP_NEW(UpdateRigidBodyMembers, urbm,());
    IMP_NEW(AccumulateRigidBodyDerivatives, arbd, ());
    Pointer<Constraint> c0
        = IMP::internal::create_tuple_constraint(urbm.get(), arbd.get(),
                                                 get_particle(),
                                                 get_particle()->get_name()
                                                 +" rigid body positions");
    get_model()->add_score_state(c0);
    get_model()->add_attribute(get_rb_score_state_0_key(),
                               get_particle_index(), c0);
  }
}



algebra::VectorD<4> RigidBody::get_rotational_derivatives() const {
  algebra::VectorD<4>
    v(get_particle()
      ->get_derivative(internal::rigid_body_data().quaternion_[0]),
      get_particle()
      ->get_derivative(internal::rigid_body_data().quaternion_[1]),
      get_particle()
      ->get_derivative(internal::rigid_body_data().quaternion_[2]),
      get_particle()
      ->get_derivative(internal::rigid_body_data().quaternion_[3]));
  return v;
}

bool RigidBody::get_coordinates_are_optimized() const {
  for (unsigned int i=0; i< 4; ++i) {
    if(!get_particle()
       ->get_is_optimized(internal::rigid_body_data().quaternion_[i]))
      return false;
  }
  return XYZ::get_coordinates_are_optimized();
}

void RigidBody::set_coordinates_are_optimized(bool tf) {
  const bool body=tf;
  const bool member=false;
  for (unsigned int i=0; i< 4; ++i) {
    get_particle()->set_is_optimized(internal::rigid_body_data().quaternion_[i],
                                     body);
  }
  XYZ::set_coordinates_are_optimized(body);
  for (unsigned int i=0; i< get_number_of_members(); ++i) {
    get_member(i).set_coordinates_are_optimized(member);
  }
}

RigidMember RigidBody::get_member(unsigned int i) const {
  IMP_USAGE_CHECK(i < get_number_of_members(),
                  "Out of range member requested: " << i
                  << " of " << get_number_of_members());
  unsigned int sz=get_member_particle_indexes().size();
  if (i < sz) {
    return RigidMember(get_model(), get_member_particle_indexes()[i]);
  } else {
    return RigidMember(get_model(),
                       get_body_member_particle_indexes()[i-sz]);
  }
}

algebra::Vector3D RigidBody::get_coordinates(RigidMember p)
  const {
  algebra::Vector3D lp= p.get_internal_coordinates();
  return get_reference_frame().get_global_coordinates(lp);
}

void RigidBody
::set_reference_frame_lazy(const IMP::algebra::ReferenceFrame3D &tr) {
  algebra::VectorD<4> v
    = tr.get_transformation_to().get_rotation().get_quaternion();
  get_particle()->set_value(internal::rigid_body_data().quaternion_[0], v[0]);
  get_particle()->set_value(internal::rigid_body_data().quaternion_[1], v[1]);
  get_particle()->set_value(internal::rigid_body_data().quaternion_[2], v[2]);
  get_particle()->set_value(internal::rigid_body_data().quaternion_[3], v[3]);
  set_coordinates(tr.get_transformation_to().get_translation());
}

void RigidBody
  ::set_reference_frame(const IMP::algebra::ReferenceFrame3D &tr) {
  set_reference_frame_lazy(tr);
  {
    const ParticleIndexes &members= get_member_particle_indexes();
    for (unsigned int i=0; i< members.size(); ++i) {
      XYZ(get_model(), members[i])
        .set_coordinates(tr.get_global_coordinates(RigidMember(get_model(),
                                                               members[i])
                                             .get_internal_coordinates()));
    }
  }
  {
    const ParticleIndexes &members= get_body_member_particle_indexes();
    for (unsigned int i=0; i< members.size(); ++i) {
      XYZ(get_model(), members[i])
        .set_coordinates(tr.get_global_coordinates(RigidMember(get_model(),
                                                               members[i])
                                             .get_internal_coordinates()));
    }
  }
}

void RigidBody::add_to_derivatives(const algebra::Vector3D &deriv_local,
                                   const algebra::Vector3D &deriv_global,
                                   const algebra::Vector3D &local,
                                   const algebra::Rotation3D &rot,
                                   DerivativeAccumulator &da) {
  //const algebra::Vector3D deriv_global= rot*deriv_local;
  //IMP_LOG_TERSE( "Accumulating rigid body derivatives" << std::endl);
  algebra::VectorD<4> q(0,0,0,0);
  for (unsigned int j=0; j< 4; ++j) {
    algebra::Vector3D v= rot.get_derivative(local, j);
    q[j]= deriv_global*v;
  }
  XYZ::add_to_derivatives(deriv_global, da);
  for (unsigned int j=0; j< 4; ++j) {
    get_model()->add_to_derivative(internal::rigid_body_data()
                                   .quaternion_[j], get_particle_index(),
                                   q[j],da);
  }
  algebra::Vector3D torque= algebra::get_vector_product(local, deriv_local);
  for (unsigned int i=0; i< 3; ++i) {
    get_model()->add_to_derivative(internal::rigid_body_data().torque_[i],
                                   get_particle_index(),
                                   torque[i], da);
  }
}

void RigidBody::add_to_derivatives(const algebra::Vector3D &deriv_local,
                                   const algebra::Vector3D &local,
                                   DerivativeAccumulator &da) {
  algebra::Rotation3D rot= get_reference_frame()
    .get_transformation_to().get_rotation();
  const algebra::Vector3D deriv_global= rot*deriv_local;
  add_to_derivatives(deriv_local, deriv_global, local, rot, da);
}


RigidBody::~RigidBody(){}
RigidMember::~RigidMember(){}
NonRigidMember::~NonRigidMember(){}






void RigidBody::show(std::ostream &out) const {
  out << "Rigid body " << get_reference_frame()
      << "("
      << get_particle()->get_derivative(internal::rigid_body_data()
                                        .quaternion_[0])
      << " "
      << get_particle()->get_derivative(internal::rigid_body_data()
                                        .quaternion_[1])
      << " "
      << get_particle()->get_derivative(internal::rigid_body_data()
                                        .quaternion_[2])
      << " "
      << get_particle()->get_derivative(internal::rigid_body_data()
                                        .quaternion_[3])
      << ")";
}

void RigidMember::show(std::ostream &out) const {
  out << "Member of " << get_rigid_body()->get_name()
      << " at " << get_internal_coordinates();
}


RigidBody RigidMember::get_rigid_body() const {
  return RigidBody(get_particle()
                   ->get_value(internal::rigid_body_data().body_));
}

void NonRigidMember::show(std::ostream &out) const {
  out << "Member of " << get_rigid_body()->get_name()
      << " at " << get_internal_coordinates();
}


RigidBody NonRigidMember::get_rigid_body() const {
  return RigidBody(get_particle()
                   ->get_value(internal::rigid_body_data().non_body_));
}




bool RigidMembersRefiner::get_can_refine(Particle *p) const {
  return RigidBody::particle_is_instance(p);
}
unsigned int RigidMembersRefiner::get_number_of_refined(Particle *p) const {
  return RigidBody(p).get_number_of_members();
}
Particle* RigidMembersRefiner::get_refined(Particle *p, unsigned int i) const {
  return RigidBody(p).get_member(i);
}

ParticlesTemp RigidMembersRefiner::get_input_particles(Particle *p) const {
  return ParticlesTemp(1,p);
}

ContainersTemp RigidMembersRefiner::get_input_containers(Particle *) const {
  return ContainersTemp();
}

void RigidMembersRefiner::do_show(std::ostream &) const {
}


namespace internal {
  RigidMembersRefiner* get_rigid_members_refiner() {
    static IMP::OwnerPointer<RigidMembersRefiner> pt
      = new RigidMembersRefiner("The rigid members refiner");
    return pt;
  }
}


namespace {
bool check_rigid_body(Particle*p) {
  algebra::Vector4D
    v(p->get_value(internal::rigid_body_data().quaternion_[0]),
      p->get_value(internal::rigid_body_data().quaternion_[1]),
      p->get_value(internal::rigid_body_data().quaternion_[2]),
      p->get_value(internal::rigid_body_data().quaternion_[3])
      );
  if (std::abs(v.get_magnitude()-1) >.1) {
    IMP_THROW("Bad quaternion in rigid body: " << v, ValueException);
  }
  return true;
}
}

IMP_CHECK_DECORATOR(RigidBody, check_rigid_body);


algebra::ReferenceFrame3D
get_initial_reference_frame(const ParticlesTemp &ps) {
  if (ps.size()==1) {
    if (RigidBody::particle_is_instance(ps[0])) {
      return RigidBody(ps[0]).get_reference_frame();
    } else {
      algebra::Transformation3D tr(algebra::get_identity_rotation_3d(),
                                   XYZ(ps[0]).get_coordinates());
      return algebra::ReferenceFrame3D(tr);
    }
  }
  IMP_USAGE_CHECK(!ps.empty(),
                  "There must be particles to make a reference frame");

  // compute center of mass
  algebra::Vector3D v(0,0,0);
  double mass=0;
  for (unsigned int i=0; i< ps.size(); ++i) {
    double cmass= 1.0;
    if (XYZR::particle_is_instance(ps[i])
        && XYZR(ps[i]).get_radius()>0) {
      cmass= std::pow(XYZR(ps[i]).get_radius(), 3);
    }
    v+= XYZ(ps[i]).get_coordinates()*cmass /*cm.get_mass()*/;
    mass+= cmass /*cm.get_mass()*/;
  }
  IMP_USAGE_CHECK(mass >0, "Zero mass when computing axis.");
  v/= mass;
  //IMP_LOG_VERBOSE( "Center of mass is " << v << std::endl);
  // for a sphere 2/5 m r^2 (diagopnal)
  // parallel axis theorem
  // I'ij= Iij+M(v^2delta_ij-vi*vj)
  // compute I
  Matrix I = compute_I(IMP::internal::get_model(ps),
                       IMP::internal::get_index(ps),
                       v, IMP::algebra::get_identity_rotation_3d());
  //IMP_LOG_VERBOSE( "Initial I is " << I << std::endl);
  // diagonalize it
  IMP::algebra::internal::JAMA::Eigenvalue<double> eig(I);
  Matrix rm;
  eig.getV(rm);
  if (IMP::algebra::internal::JAMA::determinant(rm) <0) {
    for (unsigned int i=0; i< 3; ++i) {
      for (unsigned int j=0; j< 3; ++j) {
        rm[i][j]= -rm[i][j];
      }
    }
  }
  // use the R as the initial orientation
  IMP::algebra::Rotation3D rot
    = IMP::algebra::get_rotation_from_matrix(rm[0][0], rm[0][1], rm[0][2],
                                         rm[1][0], rm[1][1], rm[1][2],
                                         rm[2][0], rm[2][1], rm[2][2]);
  //IMP_LOG_VERBOSE( "Initial rotation is " << rot << std::endl);
  return algebra::ReferenceFrame3D(
      algebra::Transformation3D(rot, v));
}


ParticlesTemp create_rigid_bodies(Model *m,
                                  unsigned int n,
                                  bool no_members) {
  ParticlesTemp ret(n);
  for (unsigned int i=0; i< n; ++i) {
    IMP_NEW(Particle, p, (m));
    ret[i]=p;
    RigidBody::setup_particle(p, algebra::ReferenceFrame3D());
  }
  IMP_NEW(IMP::internal::InternalListSingletonContainer, list, (m,
                                                       "rigid body list"));
  list->set(IMP::internal::get_index(ret));
  if (!no_members) {
    IMP_NEW(UpdateRigidBodyMembers, urbm,());
    IMP_NEW(AccumulateRigidBodyDerivatives, arbd, ());
    Pointer<Constraint> c0
        = IMP::internal::create_container_constraint(urbm.get(), arbd.get(),
                                      list.get(),
                                      "rigid body positions %1%");
    m->add_score_state(c0);
    for (unsigned int i=0; i< ret.size(); ++i) {
      m->add_attribute(get_rb_score_state_0_key(), ret[i]->get_index(), c0);
    }
  }
  return ret;
}


IMPCORE_END_NAMESPACE
