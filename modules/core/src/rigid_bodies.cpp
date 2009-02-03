/**
 *  \file rigid_bodies.cpp
 *  \brief Support for rigid bodies.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/core/rigid_bodies.h"
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/internal/tnt_array2d.h>
#include <IMP/algebra/internal/tnt_array2d_utils.h>
#include <IMP/algebra/internal/jama_eig.h>
#include <IMP/algebra/geometric_alignment.h>

IMPCORE_BEGIN_NAMESPACE


RigidBodyTraits::RigidBodyTraits(std::string pre,
                                 FloatKey mass,
                                 FloatKey radius) {
  d_= new internal::RigidBodyData();
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
  IMP_assert(get_has_required_attributes_for_body(p),
             "Particles must already be XYZDecorator particles "
             << *p);
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
                                              ParticleRefiner *gc,
                                              RigidBodyTraits tr){
  IMP_check(!tr.get_has_required_attributes_for_body(p),
            "The RigidBody is already set up.",
            InvalidStateException);
  tr.add_required_attributes_for_body(p);

  std::vector<RigidMemberDecorator> ds;
  RigidBodyDecorator d(p, tr);

  Particles members= gc->get_refined(p);
  for (unsigned int i=0; i< members.size(); ++i) {
    Particle *mp= members[i];

    IMP_check(!tr.get_has_required_attributes_for_member(p),
              "Particle " << p->get_name() << " is already part of "
              << "a conflicting rigid body",
              InvalidStateException);
    tr.add_required_attributes_for_member(mp);
    ds.push_back(RigidMemberDecorator(mp, tr));
  }
  gc->cleanup_refined(p, members);

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


IMP::algebra::Transformation3D
RigidBodyDecorator::get_transformation() const {
  IMP::algebra::Rotation3D rot(get_particle()
                               ->get_value(traits_.get_quaternion_keys()[0]),
                               get_particle()
                               ->get_value(traits_.get_quaternion_keys()[1]),
                               get_particle()
                               ->get_value(traits_.get_quaternion_keys()[2]),
                               get_particle()
                               ->get_value(traits_.get_quaternion_keys()[3]));
  return IMP::algebra::Transformation3D(rot, get_coordinates());
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







void RigidBodyDecorator::set_transformation(ParticleRefiner *gc) {
  std::vector<algebra::Vector3D> cur, local;
  Particles members= gc->get_refined(get_particle());
  for (unsigned int i=0; i< members.size(); ++i) {
    Particle *p =members[i];
    RigidMemberDecorator d(p);
    cur.push_back(d.get_coordinates());
    local.push_back(d.get_internal_coordinates());
  }
  IMP::algebra::Transformation3D tr
    = IMP::algebra::rigid_align_first_to_second(local, cur);
  IMP_LOG(VERBOSE, "Alignment is " << tr << std::endl);
  set_transformation(tr);
  for (unsigned int i=0; i< members.size(); ++i) {
    Particle *p =members[i];
    RigidMemberDecorator d(p);
    d.set_coordinates(tr.transform(d.get_internal_coordinates()));
  }
  gc->cleanup_refined(get_particle(), members);
}




RigidBodyDecorator::~RigidBodyDecorator(){}
RigidMemberDecorator::~RigidMemberDecorator(){}






void RigidBodyDecorator::show(std::ostream &out, std::string prefix) const {
  out << prefix << "Rigid body " << get_transformation()
      << std::endl;;
}

void RigidMemberDecorator::show(std::ostream &out, std::string prefix) const {
  out << prefix << "RigidMember " << get_coordinates() << " "
      << get_internal_coordinates() << std::endl;;
}



RigidBodyScoreState::RigidBodyScoreState(SingletonContainer *ps,
                                         ParticleRefiner *pr,
                                         RigidBodyTraits tr): ps_(ps),
                                                               pr_(pr),
                                                               tr_(tr){
  for (SingletonContainer::ParticleIterator pit= ps->particles_begin();
       pit != ps->particles_end(); ++pit) {
    RigidBodyDecorator::cast(*pit, tr_);
    Particles rps= pr->get_refined(*pit);
    for (unsigned int i=0; i< rps.size(); ++i) {
      Particle *p =rps[i];
      RigidMemberDecorator::cast(p, tr_);
    }
    pr->cleanup_refined(*pit, rps);
  }
}

void RigidBodyScoreState::do_before_evaluate() {
  for (SingletonContainer::ParticleIterator pit= ps_->particles_begin();
       pit != ps_->particles_end(); ++pit) {
    RigidBodyDecorator rb(*pit, tr_);
    rb.set_transformation(pr_);
  }
}



void RigidBodyScoreState::show(std::ostream &out) const {
  out << "RigidBodyScoreState " << *ps_
      << *pr_ << std::endl;
}
IMPCORE_END_NAMESPACE
