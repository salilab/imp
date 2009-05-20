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

typedef IMP::algebra::internal::TNT::Array2D<double> Matrix;

Matrix compute_I(const std::vector<RigidMember> &ds,
                 const algebra::Vector3D &center,
                 const IMP::algebra::Rotation3D &rot) {
  Matrix I(3,3, 0.0);
  for (unsigned int i=0; i< ds.size(); ++i) {
    RigidMember cm= ds[i];
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



RigidBody RigidBody::create(Particle *p,
                                              const Particles &members){
  IMP_check(!internal::get_has_required_attributes_for_body(p),
            "The RigidBody is already set up.",
            InvalidStateException);
  internal::add_required_attributes_for_body(p);

  std::vector<RigidMember> ds;
  RigidBody d(p);

  Hierarchy hd(p, internal::rigid_body_data().htraits_);

  IMP_check(!members.empty(), "There must be particles to make a rigid body",
            InvalidStateException);
  for (unsigned int i=0; i< members.size(); ++i) {
    Particle *mp= members[i];
    IMP_check(mp != p, "A rigid body cannot have itself as a member "
              << p->get_name(), ValueException);
    IMP_check(!internal::get_has_required_attributes_for_member(p),
              "Particle " << p->get_name() << " is already part of "
              << "a conflicting rigid body",
              InvalidStateException);
    internal::add_required_attributes_for_member(mp);
    ds.push_back(RigidMember(mp));
    Hierarchy hc(mp, internal::rigid_body_data().htraits_);
    hd.add_child(hc);
  }

  // compute center of mass
  algebra::Vector3D v(0,0,0);
  Float mass=0;
  for (unsigned int i=0; i< ds.size(); ++i) {
    RigidMember cm= ds[i];

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
  d.set_transformation(IMP::algebra::Transformation3D(rot, v));
  IMP_LOG(VERBOSE, "Particle is " << d << std::endl);

  for (unsigned int i=0; i< ds.size(); ++i) {
    RigidMember cm= ds[i];

    algebra::Vector3D cv=cm.get_coordinates()-v;
    algebra::Vector3D lc= roti.rotate(cv);
    cm.set_internal_coordinates(lc);
    IMP_LOG(VERBOSE, " " << cm << " | " << std::endl);
  }

  IMP_IF_CHECK(EXPENSIVE) {
    for (unsigned int i=0; i< ds.size(); ++i) {
      RigidMember cm= ds[i];
      algebra::Vector3D v= cm.get_coordinates();
      algebra::Vector3D nv= d.get_coordinates(cm);
      IMP_assert((v-nv).get_squared_magnitude() < .1,
                 "Bad initial orientation "
                 << d.get_transformation() << std::endl
                 << v << std::endl
                 << nv);
    }
  }
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


IMP::algebra::Transformation3D
RigidBody::get_transformation() const {
  algebra::VectorD<4>
    v(get_particle()->get_value(internal::rigid_body_data().quaternion_[0]),
      get_particle()->get_value(internal::rigid_body_data().quaternion_[1]),
      get_particle()->get_value(internal::rigid_body_data().quaternion_[2]),
      get_particle()->get_value(internal::rigid_body_data().quaternion_[3]));
  IMP::algebra::Rotation3D rot(v[0], v[1], v[2], v[3]);
  return IMP::algebra::Transformation3D(rot, get_coordinates());
}

RigidMembers
RigidBody::get_members() const {
  Hierarchy hd(get_particle(), internal::rigid_body_data().htraits_);
  RigidMembers rbms(hd.get_number_of_children());
  for (unsigned int i=0; i< rbms.size(); ++i) {
    rbms[i]= RigidMember(hd.get_child(i).get_particle());
  }
  return rbms;
}

unsigned int RigidBody::get_number_of_members() const {
  Hierarchy hd(get_particle(), internal::rigid_body_data().htraits_);
  return hd.get_number_of_children();
}

RigidMember RigidBody::get_member(unsigned int i) const {
  Hierarchy hd(get_particle(), internal::rigid_body_data().htraits_);
  return RigidMember(hd.get_child(i).get_particle());
}

Particles
RigidBody::get_member_particles() const {
  Hierarchy hd(get_particle(), internal::rigid_body_data().htraits_);
  return hd.get_child_particles();
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

void RigidBody::set_coordinates_are_optimized(bool tf, bool snapping) {
  bool body, member;
  if (snapping) {
    body=false;
    member= tf;
  } else {
    body=tf;
    member=false;
  }
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
::set_transformation(const IMP::algebra::Transformation3D &tr,
                     bool transform_now) {
  algebra::VectorD<4> v= tr.get_rotation().get_quaternion();
  get_particle()->set_value(internal::rigid_body_data().quaternion_[0], v[0]);
  get_particle()->set_value(internal::rigid_body_data().quaternion_[1], v[1]);
  get_particle()->set_value(internal::rigid_body_data().quaternion_[2], v[2]);
  get_particle()->set_value(internal::rigid_body_data().quaternion_[3], v[3]);
  set_coordinates(tr.get_translation());
  if (transform_now) {
    for (unsigned int i=0; i< get_number_of_members(); ++i) {
      get_member(i)
    .set_coordinates(tr.transform(get_member(i).get_internal_coordinates()));
    }
  }
}


RigidBody::~RigidBody(){}
RigidMember::~RigidMember(){}






void RigidBody::show(std::ostream &out, std::string prefix) const {
  out << prefix << "Rigid body " << get_transformation()
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

void RigidMember::show(std::ostream &out, std::string prefix) const {
  out << prefix << "Member at " << get_internal_coordinates();
}


void UpdateRigidBodyOrientation::apply(Particle *p) const {
  RigidBody rb(p);
  algebra::Vector3Ds cur, local;
  for (unsigned int i=0; i< rb.get_number_of_members(); ++i) {
    cur.push_back(rb.get_member(i).get_coordinates());
    local.push_back(rb.get_member(i).get_internal_coordinates());
  }
  IMP::algebra::Transformation3D tr
    = IMP::algebra::rigid_align_first_to_second(local, cur);
  IMP_LOG(VERBOSE, "Alignment is " << tr << std::endl);
  IMP_IF_LOG(VERBOSE) {
    IMP_LOG(VERBOSE, ".color 1 0 0\n");
    for (unsigned int i=0; i< cur.size(); ++i) {
      IMP_LOG(VERBOSE, ".sphere " << algebra::spaces_io(cur[i]) << " .1\n");
    }
    IMP_LOG(VERBOSE, ".color 0 1 0\n");
    for (unsigned int i=0; i< cur.size(); ++i) {
      IMP_LOG(VERBOSE, ".sphere " << algebra::spaces_io(tr.transform(local[i]))
              << " .1\n");
    }
  }
  rb.set_transformation(tr);
  for (unsigned int i=0; i< rb.get_number_of_members(); ++i) {
    rb.get_member(i).set_coordinates(tr.transform(rb.get_member(i)
                                            .get_internal_coordinates()));
  }
}



void UpdateRigidBodyOrientation::show(std::ostream &out) const {
  out << "RigidBodyUpdateSingletonModifier " << std::endl;
}



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



void AccumulateRigidBodyDerivatives
::show(std::ostream &out) const {
  out << "RigidBodyAccumulateDerivativesSingletonModifier " << std::endl;
}


void UpdateRigidBodyMembers::apply(Particle *p) const {
  RigidBody rb(p);
  rb.normalize_rotation();
  algebra::Transformation3D tr= rb.get_transformation();
  for (unsigned int i=0; i<rb.get_number_of_members(); ++i) {
    rb.get_member(i).set_coordinates(tr);
  }
}



void UpdateRigidBodyMembers
::show(std::ostream &out) const {
  out << "RigidBodyUpdateMembersSingletonModifier "
      << std::endl;
}


IMPCORE_END_NAMESPACE
