/**
 *  \file rigid_bodies.cpp
 *  \brief Support for rigid bodies.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/core/rigid_bodies.h"
#include "IMP/core/ListSingletonContainer.h"
#include "IMP/core/SingletonsScoreState.h"
#include "IMP/core/SingletonScoreState.h"
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/internal/tnt_array2d.h>
#include <IMP/algebra/internal/tnt_array2d_utils.h>
#include <IMP/algebra/internal/jama_eig.h>
#include <IMP/algebra/geometric_alignment.h>
#include <IMP/SingletonContainer.h>
#include <IMP/core/FixedRefiner.h>

IMPCORE_BEGIN_INTERNAL_NAMESPACE
 const RigidBodyData &rigid_body_data() {
    static RigidBodyData rbd;
    return rbd;
  }
IMPCORE_END_INTERNAL_NAMESPACE
IMPCORE_BEGIN_NAMESPACE

namespace {



void cover_rigid_body(core::RigidBody d, Refiner *ref) {
  double md=0;
  // make sure it gets cleaned up properly
  IMP::internal::OwnerPointer<Refiner> rp(ref);
  for (unsigned int i=0; i< ref->get_number_of_refined(d); ++i) {
    core::RigidMember rm(ref->get_refined(d,i));
    double cd= rm.get_internal_coordinates().get_magnitude();
    if (rm.get_particle()->has_attribute(XYZR::get_default_radius_key())) {
      cd+= rm.get_particle()->get_value(XYZR::get_default_radius_key());
    }
    md=std::max(cd, md);
  }
  if (d.get_particle()->has_attribute(XYZR::get_default_radius_key())) {
    d.get_particle()->set_value(XYZR::get_default_radius_key(), md);
  } else {
    d.get_particle()->add_attribute(XYZR::get_default_radius_key(), md);
  }
}



  ParticlesTemp get_rigid_body_used_particles(Particle *p) {
    RigidBody b(p);
    unsigned int n=b.get_number_of_members();
    ParticlesTemp ret(1+n);
    ret[0]=p;
    for (unsigned int i=0; i< n; ++i) {
      ret[i+1]= b.get_member(i);
    }
    return ret;
  }

  ParticlesList get_rigid_body_interacting_particles(Particle *p) {
    return ParticlesList(1, get_rigid_body_used_particles(p));
  }

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
  public SingletonModifier {
 public:
  AccumulateRigidBodyDerivatives(){}
  IMP_SINGLETON_MODIFIER_DA(AccumulateRigidBodyDerivatives,
                            get_module_version_info());
};


//! Compute the coordinates of the RigidMember objects bases on the orientation
/** This should be applied after evaluate to keep the bodies rigid. You can
    use the setup_rigid_bodies and setup_rigid_body methods instead of
    creating these objects yourself.
    \see setup_rigid_bodies
    \see setup_rigid_body
    \see RigidBody
    \see AccumulateRigidBodyDerivatives */
class UpdateRigidBodyMembers: public SingletonModifier {
 public:
  UpdateRigidBodyMembers(){}
  IMP_SINGLETON_MODIFIER(UpdateRigidBodyMembers,
                         get_module_version_info());
};



void AccumulateRigidBodyDerivatives::apply(Particle *p,
                                           DerivativeAccumulator &da) const {
  RigidBody rb(p);
  algebra::Rotation3D rot= rb.get_transformation().get_rotation();
  IMP_LOG(TERSE, "Accumulating rigid body derivatives" << std::endl);
  algebra::Vector3D v(0,0,0);
  algebra::VectorD<4> q(0,0,0,0);
  for (unsigned int i=0; i< rb.get_number_of_members(); ++i) {
    RigidMember d= rb.get_member(i);
    algebra::Vector3D dv= d.get_derivatives();
    v+=dv;
    IMP_LOG(TERSE, "Adding " << dv << " to derivative" << std::endl);
    for (unsigned int j=0; j< 4; ++j) {
      algebra::Vector3D v= rot.get_derivative(d.get_internal_coordinates(),
                                              j);
      IMP_LOG(VERBOSE, "Adding " << dv*v << " to quaternion deriv " << j
              << std::endl);
      q[j]+= dv*v;
    }
  }
  static_cast<XYZ>(rb).add_to_derivatives(v, da);
  for (unsigned int j=0; j< 4; ++j) {
    rb.get_particle()->add_to_derivative(internal::rigid_body_data()
                                         .quaternion_[j], q[j],da);
  }

  IMP_LOG(TERSE, "Derivative is "
          << p->get_derivative(internal::rigid_body_data().quaternion_[0])
          << " "
          << p->get_derivative(internal::rigid_body_data().quaternion_[1])
          << " "
          << p->get_derivative(internal::rigid_body_data().quaternion_[2])
          << " "
          << p->get_derivative(internal::rigid_body_data().quaternion_[3])
          << std::endl);

  IMP_LOG(TERSE, "Translation deriv is "
          << static_cast<XYZ>(rb).get_derivatives()
          << "" << std::endl);
}


void UpdateRigidBodyMembers::apply(Particle *p) const {
  RigidBody rb(p);
  rb.update_members();
}


IMP_SINGLETON_MODIFIER_FROM_REFINED(AccumulateRigidBodyDerivatives,
                                    internal::get_rigid_members_refiner());

IMP_SINGLETON_MODIFIER_TO_REFINED(UpdateRigidBodyMembers,
                                  internal::get_rigid_members_refiner());

}

typedef IMP::algebra::internal::TNT::Array2D<double> Matrix;

Matrix compute_I(const XYZs &ds,
                 const algebra::Vector3D &center,
                 const IMP::algebra::Rotation3D &rot) {
  Matrix I(3,3, 0.0);
  for (unsigned int i=0; i< ds.size(); ++i) {
    XYZ cm= ds[i];
    double m=1;
    double r=0;
    algebra::Vector3D cv=rot.rotate(cm.get_coordinates()-center);

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


IMP_SCORE_STATE_DECORATOR_DEF(RigidBody);

RigidBody RigidBody::setup_particle(Particle *p,
                                    const algebra::Transformation3D &tr) {
  internal::add_required_attributes_for_body(p);
  RigidBody rb(p);
  rb.set_transformation(tr);
  return rb;
}


RigidBody RigidBody::setup_particle(Particle *p,
                                    const XYZs &members){
  IMP_USAGE_CHECK(!internal::get_has_required_attributes_for_body(p),
            "The RigidBody is already set up.",
            InvalidStateException);

  XYZs ds;
  IMP_USAGE_CHECK(!members.empty(),
                  "There must be particles to make a rigid body",
            InvalidStateException);
  for (unsigned int i=0; i< members.size(); ++i) {
    Particle *mp= members[i];
    IMP_USAGE_CHECK(mp != p, "A rigid body cannot have itself as a member "
              << p->get_name(), ValueException);
    IMP_USAGE_CHECK(!internal::get_has_required_attributes_for_member(p),
              "Particle " << p->get_name() << " is already part of "
              << "a conflicting rigid body",
              InvalidStateException);
    ds.push_back(XYZ(mp));
  }

  // compute center of mass
  algebra::Vector3D v(0,0,0);
  Float mass=0;
  for (unsigned int i=0; i< ds.size(); ++i) {
    XYZ cm= ds[i];

    v+= cm.get_coordinates()*1.0 /*cm.get_mass()*/;
    mass+= 1.0 /*cm.get_mass()*/;
  }
  v/= mass;
  IMP_LOG(VERBOSE, "Center of mass is " << v << std::endl);
  // for a sphere 2/5 m r^2 (diagopnal)
  // parallel axis theorem
  // I'ij= Iij+M(v^2delta_ij-vi*vj)
  // compute I
  Matrix I = compute_I(ds, v, IMP::algebra::identity_rotation());
  IMP_LOG(VERBOSE, "Initial I is " << I << std::endl);
  // diagonalize it
  IMP::algebra::internal::JAMA::Eigenvalue<double> eig(I);
  Matrix rm;
  eig.getV(rm);
  // use the R as the initial orientation
  IMP::algebra::Rotation3D rot
    = IMP::algebra::rotation_from_matrix(rm[0][0], rm[0][1], rm[0][2],
                                         rm[1][0], rm[1][1], rm[1][2],
                                         rm[2][0], rm[2][1], rm[2][2]);
  IMP_LOG(VERBOSE, "Initial rotation is " << rot << std::endl);
  IMP::algebra::Rotation3D roti= rot.get_inverse();

  Matrix I2= compute_I(ds, v, roti);
  IMP_LOG(VERBOSE, I << std::endl);
  IMP_LOG(VERBOSE, I2 << std::endl);
  RigidBody d= setup_particle(p, IMP::algebra::Transformation3D(rot, v));
  IMP_LOG(VERBOSE, "Particle is " << d << std::endl);

  for (unsigned int i=0; i< ds.size(); ++i) {
    d.add_member_internal(ds[i], roti, -v);
    //IMP_LOG(VERBOSE, " " << cm << " | " << std::endl);
  }

  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    for (unsigned int i=0; i< ds.size(); ++i) {
      RigidMember cm= RigidMember(ds[i]);
      algebra::Vector3D v= cm.get_coordinates();
      algebra::Vector3D nv= d.get_coordinates(cm);
      IMP_INTERNAL_CHECK((v-nv).get_squared_magnitude() < .1,
                 "Bad initial orientation "
                 << d.get_transformation() << std::endl
                 << v << std::endl
                 << nv);
    }
  }
  set_score_state(new UpdateRigidBodyMembers(),
                  new AccumulateRigidBodyDerivatives(), p);
  cover_rigid_body(d, internal::get_rigid_members_refiner());
  return d;
}

void
RigidBody::normalize_rotation() {
  algebra::VectorD<4>
    v(get_particle()->get_value(internal::rigid_body_data().quaternion_[0]),
      get_particle()->get_value(internal::rigid_body_data().quaternion_[1]),
      get_particle()->get_value(internal::rigid_body_data().quaternion_[2]),
      get_particle()->get_value(internal::rigid_body_data().quaternion_[3]));
  IMP_LOG(TERSE, "Rotation was " << v << std::endl);
  if (v.get_squared_magnitude() >0){
    v= v.get_unit_vector();
  } else {
    v= algebra::VectorD<4>(1,0,0,0);
  }
  IMP_LOG(TERSE, "Rotation is " << v << std::endl);
  get_particle()->set_value(internal::rigid_body_data().quaternion_[0], v[0]);
  get_particle()->set_value(internal::rigid_body_data().quaternion_[1], v[1]);
  get_particle()->set_value(internal::rigid_body_data().quaternion_[2], v[2]);
  get_particle()->set_value(internal::rigid_body_data().quaternion_[3], v[3]);
}


void RigidBody::update_members() {
  normalize_rotation();
  algebra::Transformation3D tr= get_transformation();
  Hierarchy hd(get_particle(), internal::rigid_body_data().htraits_);
  for (unsigned int i=0; i< hd.get_number_of_children(); ++i) {
    RigidMember rm(hd.get_child(i));
    rm.set_coordinates(tr.transform(rm.get_internal_coordinates()));
  }
  Hierarchy hdb(get_particle(), internal::rigid_body_data().hbtraits_);
  for (unsigned int i=0; i< hdb.get_number_of_children(); ++i) {
    RigidMember rm(hdb.get_child(i));
    RigidBody rb(rm);
    rb.set_transformation(tr*rm.get_internal_transformation());
  }
}


IMP::algebra::Transformation3D
RigidBody::get_transformation() const {
  IMP::algebra::Rotation3D
    rot(get_particle()->get_value(internal::rigid_body_data().quaternion_[0]),
        get_particle()->get_value(internal::rigid_body_data().quaternion_[1]),
        get_particle()->get_value(internal::rigid_body_data().quaternion_[2]),
        get_particle()->get_value(internal::rigid_body_data().quaternion_[3]));
  return IMP::algebra::Transformation3D(rot, get_coordinates());
}

RigidMembers
RigidBody::get_members() const {
  Hierarchy hd(get_particle(), internal::rigid_body_data().htraits_);
  RigidMembers rbms(hd.get_number_of_children());
  for (unsigned int i=0; i< rbms.size(); ++i) {
    rbms.set(i, RigidMember(hd.get_child(i)));
  }
  return rbms;
}

unsigned int RigidBody::get_number_of_members() const {
  Hierarchy hd(get_particle(), internal::rigid_body_data().htraits_);
  Hierarchy hdb(get_particle(), internal::rigid_body_data().hbtraits_);
  return hd.get_number_of_children()+hdb.get_number_of_children();
}

RigidMember RigidBody::get_member(unsigned int i) const {
  Hierarchy hd(get_particle(), internal::rigid_body_data().htraits_);
  if (i < hd.get_number_of_children()) {
    return RigidMember(hd.get_child(i).get_particle());
  } else {
    Hierarchy hdb(get_particle(), internal::rigid_body_data().hbtraits_);
    return RigidMember(hdb.get_child(i
                              -hd.get_number_of_children()).get_particle());
  }
}

void RigidBody::add_member(XYZ d) {
  algebra::Transformation3D tr= get_transformation();
  algebra::Rotation3D roti= tr.get_rotation().get_inverse();
  add_member_internal(d, roti, -tr.get_translation());
}

void RigidBody::add_member_internal(XYZ d, const algebra::Rotation3D &roti,
                                    const algebra::Vector3D &transi) {
  internal::add_required_attributes_for_member(d);
  RigidMember cm(d);
  Hierarchy hc(d, internal::rigid_body_data().htraits_);
  Hierarchy hd(*this, internal::rigid_body_data().htraits_);
  hd.add_child(hc);
  algebra::Vector3D cv=cm.get_coordinates()+transi;
  algebra::Vector3D lc= roti.rotate(cv);
  cm.set_internal_coordinates(lc);
  cover_rigid_body(*this, internal::get_rigid_members_refiner());
}

void RigidBody::add_member(RigidBody d) {
  algebra::Transformation3D tri= get_transformation().get_inverse();
  internal::add_required_attributes_for_body_member(d);
  RigidMember cm(d);
  Hierarchy hc(d, internal::rigid_body_data().hbtraits_);
  Hierarchy hd(*this, internal::rigid_body_data().hbtraits_);
  hd.add_child(hc);
  algebra::Transformation3D btr=d.get_transformation();
  // want tr*ltr= btr, so ltr= tr-1*btr
  cm.set_internal_transformation(tri*btr);
  cover_rigid_body(*this, internal::get_rigid_members_refiner());
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

algebra::Vector3D RigidBody::get_coordinates(RigidMember p)
                                                                      const {
  algebra::Vector3D lp= p.get_internal_coordinates();
  IMP::algebra::Transformation3D tr= get_transformation();
  return tr.transform(lp);
}

void RigidBody
::lazy_set_transformation(const IMP::algebra::Transformation3D &tr) {
 algebra::VectorD<4> v= tr.get_rotation().get_quaternion();
  get_particle()->set_value(internal::rigid_body_data().quaternion_[0], v[0]);
  get_particle()->set_value(internal::rigid_body_data().quaternion_[1], v[1]);
  get_particle()->set_value(internal::rigid_body_data().quaternion_[2], v[2]);
  get_particle()->set_value(internal::rigid_body_data().quaternion_[3], v[3]);
  set_coordinates(tr.get_translation());
}

void RigidBody
::set_transformation(const IMP::algebra::Transformation3D &tr) {
  lazy_set_transformation(tr);
  for (unsigned int i=0; i< get_number_of_members(); ++i) {
    get_member(i)
      .set_coordinates(tr.transform(get_member(i).get_internal_coordinates()));
  }
}


RigidBody::~RigidBody(){}
RigidMember::~RigidMember(){}






void RigidBody::show(std::ostream &out) const {
  out << "Rigid body " << get_transformation()
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
  out << "Member at " << get_internal_coordinates();
}


RigidBody RigidMember::get_rigid_body() const {
  if (internal::get_has_required_attributes_for_body_member(*this)) {
    Hierarchy hd(*this, internal::rigid_body_data().hbtraits_);
    return RigidBody(hd.get_parent());
  } else {
    Hierarchy hc(*this, internal::rigid_body_data().htraits_);
    return RigidBody(hc.get_parent());
  }
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

void RigidMembersRefiner::show(std::ostream &out) const {
  out << "RigidMembersRefiner";
}


namespace internal {
  IMPCOREEXPORT RigidMembersRefiner* get_rigid_members_refiner() {
    static IMP::internal::OwnerPointer<RigidMembersRefiner> pt
      = new RigidMembersRefiner();
    return pt;
  }
}



IMPCORE_END_NAMESPACE
