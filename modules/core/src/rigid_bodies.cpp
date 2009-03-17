/**
 *  \file rigid_bodies.cpp
 *  \brief Support for rigid bodies.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
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
#include <IMP/core/FixedParticleRefiner.h>

IMPCORE_BEGIN_NAMESPACE


RigidBodyTraits::RigidBodyTraits(ParticleRefiner *pr,
                                 std::string pre,
                                 FloatKey mass,
                                 FloatKey radius,
                                 bool snap) {
  d_= new internal::RigidBodyData();
  d_->pr_= pr;
  d_->child_keys_.resize(3);
  d_->child_keys_[0]= FloatKey((pre+"local_x").c_str());
  d_->child_keys_[1]= FloatKey((pre+"local_y").c_str());
  d_->child_keys_[2]= FloatKey((pre+"local_z").c_str());
  d_->quaternion_.resize(4);
  d_->quaternion_[0]= FloatKey((pre+"quaternion_0").c_str());
  d_->quaternion_[1]= FloatKey((pre+"quaternion_1").c_str());
  d_->quaternion_[2]= FloatKey((pre+"quaternion_2").c_str());
  d_->quaternion_[3]= FloatKey((pre+"quaternion_3").c_str());
  d_->mass_= mass;
  d_->radius_= radius;
  d_->snap_= snap;
}

void RigidBodyTraits::set_model_ranges(Model *m) const {
  m->set_range(d_->quaternion_[0], FloatPair(0,1));
  m->set_range(d_->quaternion_[1], FloatPair(0,1));
  m->set_range(d_->quaternion_[2], FloatPair(0,1));
  m->set_range(d_->quaternion_[3], FloatPair(0,1));
}

bool RigidBodyTraits::get_has_required_attributes_for_body(Particle *p) const {
  for (unsigned int i=0; i< 4; ++i) {
    if (!p->has_attribute(d_->quaternion_[i])) return false;
  }
  for (unsigned int i=0; i< 3; ++i) {
    if (!p->has_attribute(XYZDecorator::get_xyz_keys()[i]))
      return false;
  }
  return true;
}



bool
RigidBodyTraits::get_has_required_attributes_for_member(Particle *p) const {
  for (unsigned int i=0; i< 3; ++i) {
    if (!p->has_attribute(d_->child_keys_[i])) return false;
  }
  if (d_->mass_ != FloatKey() && !p->has_attribute(d_->mass_)) return false;
  for (unsigned int i=0; i< 3; ++i) {
     if (!p->has_attribute(XYZDecorator::get_xyz_keys()[i]))
       return false;
  }
  return true;
}

void RigidBodyTraits::add_required_attributes_for_body(Particle *p) const {
  for (unsigned int i=0; i< 4; ++i) {
    p->add_attribute(d_->quaternion_[i], 0);
  }
  if (!XYZDecorator::is_instance_of(p)) {
    XYZDecorator::create(p);
  }
}

void RigidBodyTraits::add_required_attributes_for_member(Particle *p) const {
  for (unsigned int i=0; i< 3; ++i) {
    p->add_attribute(d_->child_keys_[i], 0);
  }
  IMP_check(d_->mass_ == FloatKey() || p->has_attribute(d_->mass_),
            "Particle is missing mass attribute "
            << *p, InvalidStateException);
  IMP_check(d_->radius_ == FloatKey() || p->has_attribute(d_->radius_),
            "Particle is missing radius attribute "
            << *p, InvalidStateException);
  XYZDecorator::cast(p);
}



typedef IMP::algebra::internal::TNT::Array2D<double> Matrix;

Matrix compute_I(const std::vector<RigidMemberDecorator> &ds,
                 RigidBodyTraits tr,
                 const algebra::Vector3D &center,
                 const IMP::algebra::Rotation3D &rot) {
  Matrix I(3,3, 0.0);
  for (unsigned int i=0; i< ds.size(); ++i) {
    RigidMemberDecorator cm= ds[i];
    Float r= cm.get_radius();
    Float m= cm.get_mass();
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



RigidBodyDecorator RigidBodyDecorator::create(Particle *p,
                                              RigidBodyTraits tr){
  IMP_check(!tr.get_has_required_attributes_for_body(p),
            "The RigidBody is already set up.",
            InvalidStateException);
  tr.add_required_attributes_for_body(p);

  std::vector<RigidMemberDecorator> ds;
  RigidBodyDecorator d(p, tr);

  Particles members= tr.get_particle_refiner()->get_refined(p);
  IMP_check(!members.empty(), "There must be particles to make a rigid body",
            InvalidStateException);
  for (unsigned int i=0; i< members.size(); ++i) {
    Particle *mp= members[i];

    IMP_check(!tr.get_has_required_attributes_for_member(p),
              "Particle " << p->get_name() << " is already part of "
              << "a conflicting rigid body",
              InvalidStateException);
    tr.add_required_attributes_for_member(mp);
    ds.push_back(RigidMemberDecorator(mp, tr));
  }
  tr.get_particle_refiner()->cleanup_refined(p, members);

  // compute center of mass
  algebra::Vector3D v(0,0,0);
  Float mass=0;
  for (unsigned int i=0; i< ds.size(); ++i) {
    RigidMemberDecorator cm= ds[i];

    v+= cm.get_coordinates()*cm.get_mass();
    mass+= cm.get_mass();
  }
  v/= mass;
  IMP_LOG(VERBOSE, "Center of mass is " << v << std::endl);
  // for a sphere 2/5 m r^2 (diagopnal)
  // parallel axis theorem
  // I'ij= Iij+M(v^2delta_ij-vi*vj)
  // compute I
  Matrix I = compute_I(ds, tr, v, IMP::algebra::identity_rotation());
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

  Matrix I2= compute_I(ds, tr, v, roti);
  std::cout << I << std::endl;
  std::cout << I2 << std::endl;
  d.set_transformation(IMP::algebra::Transformation3D(rot, v));
  IMP_LOG(VERBOSE, "Particle is " << d << std::endl);

  for (unsigned int i=0; i< ds.size(); ++i) {
    RigidMemberDecorator cm= ds[i];

    algebra::Vector3D cv=cm.get_coordinates()-v;
    algebra::Vector3D lc= roti.rotate(cv);
    cm.set_internal_coordinates(lc);
     IMP_LOG(VERBOSE, "Member particle is " << cm << std::endl);
  }

  IMP_IF_CHECK(EXPENSIVE) {
    for (unsigned int i=0; i< ds.size(); ++i) {
      RigidMemberDecorator cm= ds[i];
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
RigidBodyDecorator::normalize_rotation() {
  algebra::VectorD<4> v(get_particle()
                        ->get_value(traits_.get_quaternion_keys()[0]),
                        get_particle()
                        ->get_value(traits_.get_quaternion_keys()[1]),
                        get_particle()
                        ->get_value(traits_.get_quaternion_keys()[2]),
                        get_particle()
                        ->get_value(traits_.get_quaternion_keys()[3]));
  IMP_LOG(TERSE, "Rotation was " << v << std::endl);
  if (v.get_squared_magnitude() >0){
    v= v.get_unit_vector();
  } else {
    v= algebra::VectorD<4>(1,0,0,0);
  }
  IMP_LOG(TERSE, "Rotation is " << v << std::endl);
  get_particle()->set_value(traits_.get_quaternion_keys()[0], v[0]);
  get_particle()->set_value(traits_.get_quaternion_keys()[1], v[1]);
  get_particle()->set_value(traits_.get_quaternion_keys()[2], v[2]);
  get_particle()->set_value(traits_.get_quaternion_keys()[3], v[3]);
}


IMP::algebra::Transformation3D
RigidBodyDecorator::get_transformation() const {
  algebra::VectorD<4> v(get_particle()
                        ->get_value(traits_.get_quaternion_keys()[0]),
                        get_particle()
                        ->get_value(traits_.get_quaternion_keys()[1]),
                        get_particle()
                        ->get_value(traits_.get_quaternion_keys()[2]),
                        get_particle()
                        ->get_value(traits_.get_quaternion_keys()[3]));
  IMP::algebra::Rotation3D rot(v[0], v[1], v[2], v[3]);
  return IMP::algebra::Transformation3D(rot, get_coordinates());
}

RigidMemberDecorators
RigidBodyDecorator::get_members() const {
  Particles r= get_traits().get_particle_refiner()->get_refined(get_particle());
  RigidMemberDecorators rbms(r.size());
  for (unsigned int i=0; i< r.size(); ++i) {
    rbms[i]= RigidMemberDecorator(r[i], get_traits());
  }
  return rbms;
}

algebra::VectorD<4> RigidBodyDecorator::get_rotational_derivatives() const {
  algebra::VectorD<4> v(get_particle()
                        ->get_derivative(traits_.get_quaternion_keys()[0]),
                        get_particle()
                        ->get_derivative(traits_.get_quaternion_keys()[1]),
                        get_particle()
                        ->get_derivative(traits_.get_quaternion_keys()[2]),
                        get_particle()
                        ->get_derivative(traits_.get_quaternion_keys()[3]));
  return v;
}

void RigidBodyDecorator::set_coordinates_are_optimized(bool tf) {
  bool body, member;
  if (get_traits().get_snapping()) {
    body=false;
    member= tf;
  } else {
    body=tf;
    member=false;
  }
  for (unsigned int i=0; i< 4; ++i) {
    get_particle()->set_is_optimized(traits_.get_quaternion_keys()[i], body);
  }
  XYZDecorator::set_coordinates_are_optimized(body);
  RigidMemberDecorators rmds=get_members();
  for (unsigned int i=0; i< rmds.size(); ++i) {
    rmds[i].set_coordinates_are_optimized(member);
  }
}

algebra::Vector3D RigidBodyDecorator::get_coordinates(RigidMemberDecorator p)
                                                                      const {
  algebra::Vector3D lp= p.get_internal_coordinates();
  IMP::algebra::Transformation3D tr= get_transformation();
  return tr.transform(lp);
}

void RigidBodyDecorator
::set_transformation(const IMP::algebra::Transformation3D &tr) {
  algebra::VectorD<4> v= tr.get_rotation().get_quaternion();
  get_particle()->set_value(get_traits().get_quaternion_keys()[0], v[0]);
  get_particle()->set_value(get_traits().get_quaternion_keys()[1], v[1]);
  get_particle()->set_value(get_traits().get_quaternion_keys()[2], v[2]);
  get_particle()->set_value(get_traits().get_quaternion_keys()[3], v[3]);
  set_coordinates(tr.get_translation());
}


RigidBodyDecorator::~RigidBodyDecorator(){}
RigidMemberDecorator::~RigidMemberDecorator(){}






void RigidBodyDecorator::show(std::ostream &out, std::string prefix) const {
  out << prefix << "Rigid body " << get_transformation()
      << "("
      << get_particle()->get_derivative(get_traits().get_quaternion_keys()[0])
      << " "
      << get_particle()->get_derivative(get_traits().get_quaternion_keys()[1])
      << " "
      << get_particle()->get_derivative(get_traits().get_quaternion_keys()[2])
      << " "
      << get_particle()->get_derivative(get_traits().get_quaternion_keys()[3])
      << ")"
      << std::endl;;
}

void RigidMemberDecorator::show(std::ostream &out, std::string prefix) const {
  out << prefix << "RigidMember " << get_coordinates() << " "
      << get_internal_coordinates() << std::endl;;
}


void UpdateRigidBodyOrientation::apply(Particle *p) const {
  RigidBodyDecorator rb(p, tr_);
  algebra::Vector3Ds cur, local;
  Particles members= tr_.get_particle_refiner()->get_refined(p);
  for (unsigned int i=0; i< members.size(); ++i) {
    Particle *p =members[i];
    RigidMemberDecorator d(p, tr_);
    cur.push_back(d.get_coordinates());
    local.push_back(d.get_internal_coordinates());
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
  for (unsigned int i=0; i< members.size(); ++i) {
    Particle *p =members[i];
    RigidMemberDecorator d(p, tr_);
    d.set_coordinates(tr.transform(d.get_internal_coordinates()));
  }
  tr_.get_particle_refiner()->cleanup_refined(p, members);
}



void UpdateRigidBodyOrientation::show(std::ostream &out) const {
  out << "RigidBodyUpdateSingletonModifier " << tr_ << std::endl;
}



void AccumulateRigidBodyDerivatives::apply(Particle *p,
                                           DerivativeAccumulator *da) const {
  if (!da) return;
  RigidBodyDecorator rb(p, tr_);
  Particles members= tr_.get_particle_refiner()->get_refined(p);
  algebra::Rotation3D rot= rb.get_transformation().get_rotation();
  IMP_LOG(TERSE, "Accumulating rigid body derivatives" << std::endl);
  algebra::Vector3D v(0,0,0);
  for (unsigned int i=0; i< members.size(); ++i) {
    Particle *mp =members[i];
    RigidMemberDecorator d(mp, tr_);
    algebra::Vector3D dv= d.get_derivatives();
    v+=dv;
    IMP_LOG(TERSE, "Adding " << dv << " to derivative" << std::endl);
    for (unsigned int i=0; i< 4; ++i) {
      algebra::Vector3D v= rot.get_derivative(d.get_internal_coordinates(), i);
      IMP_LOG(VERBOSE, "Adding " << dv*v << " to quaternion deriv " << i
              << std::endl);
      p->add_to_derivative(tr_.get_quaternion_keys()[i],
                           dv*v,
                           *da);
    }
  }
  static_cast<XYZDecorator>(rb).add_to_derivatives(v, *da);
  IMP_LOG(TERSE, "Derivative is "
          << p->get_derivative(tr_.get_quaternion_keys()[0]) << " "
          << p->get_derivative(tr_.get_quaternion_keys()[1]) << " "
          << p->get_derivative(tr_.get_quaternion_keys()[2]) << " "
          << p->get_derivative(tr_.get_quaternion_keys()[3])
          << std::endl);

  tr_.get_particle_refiner()->cleanup_refined(p, members);
  IMP_LOG(TERSE, "Translation deriv is "
          << static_cast<XYZDecorator>(rb).get_derivatives()
          << "" << std::endl);

}



void AccumulateRigidBodyDerivatives
::show(std::ostream &out) const {
  out << "RigidBodyAccumulateDerivativesSingletonModifier "
      << tr_ << std::endl;
}


void UpdateRigidBodyMembers::apply(Particle *p) const {
  RigidBodyDecorator rb(p, tr_);
  rb.normalize_rotation();
  Particles members= tr_.get_particle_refiner()->get_refined(p);
  algebra::Transformation3D tr= rb.get_transformation();
  for (unsigned int i=0; i< members.size(); ++i) {
    Particle *p =members[i];
    RigidMemberDecorator rm(p, tr_);
    rm.set_coordinates(tr);
  }
  tr_.get_particle_refiner()->cleanup_refined(p, members);

}



void UpdateRigidBodyMembers
::show(std::ostream &out) const {
  out << "RigidBodyUpdateMembersSingletonModifier "
      << tr_ << std::endl;
}


namespace {
  typedef std::pair<SingletonModifier*, SingletonModifier*> SMP;
  SMP
  get_modifiers(RigidBodyTraits tr) {
    if (tr.get_snapping()) {
      return SMP(new UpdateRigidBodyOrientation(tr), NULL);
    } else {
      return SMP(new UpdateRigidBodyMembers(tr),
                 new AccumulateRigidBodyDerivatives(tr));
    }
  }
}

void create_rigid_bodies(SingletonContainer *rbs,
                         RigidBodyTraits tr) {
  IMP_check(rbs->get_number_of_particles() >0,
            "Need some particles to make rigid bodies",
            ValueException);
  Model *m= rbs->get_particle(0)->get_model();
  for (SingletonContainer::ParticleIterator pit= rbs->particles_begin();
       pit != rbs->particles_end();++pit) {
    RigidBodyDecorator rbd= RigidBodyDecorator::create(*pit, tr);
    rbd.set_coordinates_are_optimized(true);
  }
  SMP sm= get_modifiers(tr);
  SingletonsScoreState *sss= new SingletonsScoreState(rbs, sm.first, sm.second);
  m->add_score_state(sss);
}

RigidBodyDecorator create_rigid_body(Particle *p,
                                     RigidBodyTraits tr) {
  Model *m= p->get_model();
  RigidBodyDecorator rbd= RigidBodyDecorator::create(p, tr);
  SMP sm= get_modifiers(tr);
  rbd.set_coordinates_are_optimized(true);
  SingletonScoreState *sss= new SingletonScoreState(sm.first, sm.second, p);
  m->add_score_state(sss);
  return rbd;
}


IMPCORE_END_NAMESPACE
