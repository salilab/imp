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
#include <IMP/algebra/eigen_analysis.h>
#include <IMP/core/internal/Grid3D.h>
IMPCORE_BEGIN_INTERNAL_NAMESPACE
 const RigidBodyData &rigid_body_data() {
    static RigidBodyData rbd;
    return rbd;
  }
IMPCORE_END_INTERNAL_NAMESPACE
IMPCORE_BEGIN_NAMESPACE

typedef IMP::algebra::internal::TNT::Array2D<double> Matrix;

Matrix compute_I(const std::vector<RigidMemberDecorator> &ds,
                 const algebra::Vector3D &center,
                 const IMP::algebra::Rotation3D &rot) {
  Matrix I(3,3, 0.0);
  for (unsigned int i=0; i< ds.size(); ++i) {
    RigidMemberDecorator cm= ds[i];
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



RigidBodyDecorator RigidBodyDecorator::create(Particle *p,
                                              const Particles &members){
  IMP_check(!internal::get_has_required_attributes_for_body(p),
            "The RigidBody is already set up.",
            InvalidStateException);
  internal::add_required_attributes_for_body(p);

  std::vector<RigidMemberDecorator> ds;
  RigidBodyDecorator d(p);

  HierarchyDecorator hd(p, internal::rigid_body_data().htraits_);

  IMP_check(!members.empty(), "There must be particles to make a rigid body",
            InvalidStateException);
  for (unsigned int i=0; i< members.size(); ++i) {
    Particle *mp= members[i];

    IMP_check(!internal::get_has_required_attributes_for_member(p),
              "Particle " << p->get_name() << " is already part of "
              << "a conflicting rigid body",
              InvalidStateException);
    internal::add_required_attributes_for_member(mp);
    ds.push_back(RigidMemberDecorator(mp));
    HierarchyDecorator hc(mp, internal::rigid_body_data().htraits_);
    hd.add_child(hc);
  }

  // compute center of mass
  algebra::Vector3D v(0,0,0);
  Float mass=0;
  for (unsigned int i=0; i< ds.size(); ++i) {
    RigidMemberDecorator cm= ds[i];

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
RigidBodyDecorator::get_transformation() const {
  algebra::VectorD<4>
    v(get_particle()->get_value(internal::rigid_body_data().quaternion_[0]),
      get_particle()->get_value(internal::rigid_body_data().quaternion_[1]),
      get_particle()->get_value(internal::rigid_body_data().quaternion_[2]),
      get_particle()->get_value(internal::rigid_body_data().quaternion_[3]));
  IMP::algebra::Rotation3D rot(v[0], v[1], v[2], v[3]);
  return IMP::algebra::Transformation3D(rot, get_coordinates());
}

RigidMemberDecorators
RigidBodyDecorator::get_members() const {
  HierarchyDecorator hd(get_particle(), internal::rigid_body_data().htraits_);
  RigidMemberDecorators rbms(hd.get_number_of_children());
  for (unsigned int i=0; i< rbms.size(); ++i) {
    rbms[i]= RigidMemberDecorator(hd.get_child(i).get_particle());
  }
  return rbms;
}

unsigned int RigidBodyDecorator::get_number_of_members() const {
  HierarchyDecorator hd(get_particle(), internal::rigid_body_data().htraits_);
  return hd.get_number_of_children();
}

RigidMemberDecorator RigidBodyDecorator::get_member(unsigned int i) const {
  HierarchyDecorator hd(get_particle(), internal::rigid_body_data().htraits_);
  return RigidMemberDecorator(hd.get_child(i).get_particle());
}

Particles
RigidBodyDecorator::get_member_particles() const {
  HierarchyDecorator hd(get_particle(), internal::rigid_body_data().htraits_);
  return hd.get_child_particles();
}

algebra::VectorD<4> RigidBodyDecorator::get_rotational_derivatives() const {
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

bool RigidBodyDecorator::get_coordinates_are_optimized() const {
  for (unsigned int i=0; i< 4; ++i) {
    if(!get_particle()
       ->get_is_optimized(internal::rigid_body_data().quaternion_[i]))
      return false;
  }
  return XYZDecorator::get_coordinates_are_optimized();
}

void RigidBodyDecorator::set_coordinates_are_optimized(bool tf, bool snapping) {
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
  get_particle()->set_value(internal::rigid_body_data().quaternion_[0], v[0]);
  get_particle()->set_value(internal::rigid_body_data().quaternion_[1], v[1]);
  get_particle()->set_value(internal::rigid_body_data().quaternion_[2], v[2]);
  get_particle()->set_value(internal::rigid_body_data().quaternion_[3], v[3]);
  set_coordinates(tr.get_translation());
}


RigidBodyDecorator::~RigidBodyDecorator(){}
RigidMemberDecorator::~RigidMemberDecorator(){}






void RigidBodyDecorator::show(std::ostream &out, std::string prefix) const {
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
      << ")"
      << std::endl;;
}

void RigidMemberDecorator::show(std::ostream &out, std::string prefix) const {
  out << prefix << "RigidMember " << get_coordinates() << " "
      << get_internal_coordinates() << std::endl;;
}


void UpdateRigidBodyOrientation::apply(Particle *p) const {
  RigidBodyDecorator rb(p);
  algebra::Vector3Ds cur, local;
  RigidMemberDecorators members=rb.get_members();
  for (unsigned int i=0; i< members.size(); ++i) {
    cur.push_back(members[i].get_coordinates());
    local.push_back(members[i].get_internal_coordinates());
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
    members[i].set_coordinates(tr.transform(members[i]
                                            .get_internal_coordinates()));
  }
}



void UpdateRigidBodyOrientation::show(std::ostream &out) const {
  out << "RigidBodyUpdateSingletonModifier " << std::endl;
}



void AccumulateRigidBodyDerivatives::apply(Particle *p,
                                           DerivativeAccumulator &da) const {
  RigidBodyDecorator rb(p);
  RigidMemberDecorators members= rb.get_members();
  algebra::Rotation3D rot= rb.get_transformation().get_rotation();
  IMP_LOG(TERSE, "Accumulating rigid body derivatives" << std::endl);
  algebra::Vector3D v(0,0,0);
  algebra::VectorD<4> q(0,0,0,0);
  for (unsigned int i=0; i< members.size(); ++i) {
    RigidMemberDecorator d= members[i];
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
  static_cast<XYZDecorator>(rb).add_to_derivatives(v, da);
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
          << static_cast<XYZDecorator>(rb).get_derivatives()
          << "" << std::endl);
}



void AccumulateRigidBodyDerivatives
::show(std::ostream &out) const {
  out << "RigidBodyAccumulateDerivativesSingletonModifier " << std::endl;
}


void UpdateRigidBodyMembers::apply(Particle *p) const {
  RigidBodyDecorator rb(p);
  rb.normalize_rotation();
  RigidMemberDecorators members= rb.get_members();
  algebra::Transformation3D tr= rb.get_transformation();
  for (unsigned int i=0; i< members.size(); ++i) {
    members[i].set_coordinates(tr);
  }
}



void UpdateRigidBodyMembers
::show(std::ostream &out) const {
  out << "RigidBodyUpdateMembersSingletonModifier "
      << std::endl;
}


namespace {
  typedef std::pair<SingletonModifier*, SingletonModifier*> SMP;
  SMP
  get_modifiers(bool snapping) {
    if (snapping) {
      return SMP(new UpdateRigidBodyOrientation(), NULL);
    } else {
      return SMP(new UpdateRigidBodyMembers(),
                 new AccumulateRigidBodyDerivatives());
    }
  }
}

ScoreState* create_rigid_bodies(SingletonContainer *rbs,
                                Refiner *pr,
                                bool snapping) {
  IMP_check(rbs->get_number_of_particles() >0,
            "Need some particles to make rigid bodies",
            ValueException);
  for (SingletonContainer::ParticleIterator pit= rbs->particles_begin();
       pit != rbs->particles_end();++pit) {
    RigidBodyDecorator rbd= RigidBodyDecorator::create(*pit,
                                                       pr->get_refined(*pit));
    rbd.set_coordinates_are_optimized(true, snapping);
  }
  SMP sm= get_modifiers(snapping);
  SingletonsScoreState *sss= new SingletonsScoreState(rbs, sm.first, sm.second);
  return sss;
}

ScoreState* create_rigid_body(Particle *p,
                              const Particles &ps,
                              bool snapping) {
  RigidBodyDecorator rbd= RigidBodyDecorator::create(p, ps);
  SMP sm= get_modifiers(snapping);
  rbd.set_coordinates_are_optimized(true, snapping);
  SingletonScoreState *sss= new SingletonScoreState(sm.first, sm.second, p);
  return sss;
}


namespace {
  internal::SpheresSplit divide_spheres(const algebra::Sphere3Ds &ss,
                                        const internal::SphereIndexes &s) {
    algebra::Vector3D centroid(0,0,0);
  for (unsigned int i=0; i< s.size(); ++i) {
    centroid += ss[s[i]].get_center();
  }
  centroid/= s.size();
  algebra::Vector3Ds pts(s.size());
  for (unsigned int i=0; i< s.size(); ++i) {
    pts[i]= ss[s[i]].get_center()-centroid;
  }
  algebra::PrincipalComponentAnalysis pca= algebra::principal_components(pts);
  const algebra::Vector3D &v0=pca.get_principal_component(0),
    &v1= pca.get_principal_component(1),
    &v2= pca.get_principal_component(2);
  algebra::Rotation3D r= algebra::rotation_from_matrix(v0[0], v0[1], v0[2],
                                                       v1[0], v1[1], v1[2],
                                                       v2[0], v2[1], v2[2])
    .get_inverse();
  algebra::Vector3D minc(std::numeric_limits<double>::max(),
                         std::numeric_limits<double>::max(),
                         std::numeric_limits<double>::max()),
    maxc(-std::numeric_limits<double>::max(),
         -std::numeric_limits<double>::max(),
         -std::numeric_limits<double>::max());
  for (unsigned int i=0; i< s.size(); ++i) {
    pts[i]= r.rotate(pts[i]);
    for (unsigned int j=0; j< 3; ++j) {
      minc[j]= std::min(minc[j], pts[i][j]);
      maxc[j]= std::max(maxc[j], pts[i][j]);
    }
  }
  double side=std::numeric_limits<double>::max();
  for (unsigned int j=0; j< 3; ++j) {
    side= std::min(side, (maxc[j]-minc[j])/2.0);
  }
  typedef internal::Grid3D<internal::SphereIndexes > Grid;
  Grid grid(side, minc, maxc, internal::SphereIndexes());
  for (unsigned int i=0; i< s.size(); ++i) {
    Grid::Index ix= grid.get_index(pts[i]);
    grid.get_voxel(ix).push_back(s[i]);
  }

  internal::SpheresSplit ret;
  for (Grid::IndexIterator it= grid.all_indexes_begin();
       it != grid.all_indexes_end(); ++it) {
    if (!grid.get_voxel(*it).empty()) {
      ret.push_back(grid.get_voxel(*it));
    }
  }
  return ret;
}
}


namespace internal {
  RigidBodyParticleData::RigidBodyParticleData() {
    data_.push_back(Data());
  }
  void RigidBodyParticleData::set_sphere(unsigned int ni,
                                         const algebra::Sphere3D &s) {
    IMP_assert(ni < data_.size(), "Out of range");
    data_[ni].s_=s;
  }
  void RigidBodyParticleData::set_leaf(unsigned int ni,
                                       const std::vector<unsigned int> &ids) {
    IMP_assert(ni < data_.size(), "Out of range");
    data_[ni].storage_.resize(ids.size());
    for (unsigned int i=0; i< ids.size(); ++i) {
      data_[ni].storage_[i]= -ids[i]-1;
    }
  }
  unsigned int RigidBodyParticleData::add_children(unsigned int ni,
                                                   unsigned int num_children)  {
    IMP_assert(ni < data_.size(), "Out of range");
    IMP_assert(num_children >1, "Need to have children");
    unsigned int ret= data_.size();
    data_.insert(data_.end(), num_children, Data());
    data_[ni].storage_.resize(num_children);
    for (unsigned int i=0; i< num_children; ++i) {
      data_[ni].storage_[i]= ret+i;
    }
    return ret;
  }
  bool RigidBodyParticleData::get_is_leaf(unsigned int ni) const {
    IMP_assert(ni < data_.size(), "Out of range");
    IMP_assert(!data_[ni].storage_.empty(),
               "Everything must have particles or children");
    return data_[ni].storage_[0] < 0;
  }
  unsigned int
  RigidBodyParticleData::get_number_of_particles(unsigned int ni) const  {
    IMP_assert(ni < data_.size(), "Out of range");
    IMP_assert(get_is_leaf(ni), "Only leaves have particles");
    return data_[ni].storage_.size();
  }
  unsigned int
  RigidBodyParticleData::get_number_of_children(unsigned int ni) const  {
    IMP_assert(ni < data_.size(), "Out of range");
    if (!get_is_leaf(ni)) {
      return data_[ni].storage_.size();
    } else {
      return 1;
    }
  }
  unsigned int RigidBodyParticleData::get_child(unsigned int ni,
                                                unsigned int i) const  {
    IMP_assert(ni < data_.size(), "Out of range");
    IMP_assert(data_[ni].storage_.size() > i,
               "Out of range in particle");
    if (!get_is_leaf(ni)) {
      return data_[ni].storage_[i];
    } else {
      return ni;
    }
  }
  unsigned int RigidBodyParticleData::get_particle(unsigned int ni,
                                                   unsigned int i) const  {
    IMP_assert(ni < data_.size(), "Out of range");
    IMP_assert(data_[ni].storage_.size() > i,
               "Out of range in particle");
    IMP_assert(data_[ni].storage_[i] < 0,
               "Not a leaf node");
    return std::abs(data_[ni].storage_[i])-1;
  }
  const algebra::Sphere3D &
  RigidBodyParticleData::get_sphere(unsigned int ni) const  {
    IMP_assert(ni < data_.size(), "Out of range");
    return data_[ni].s_;
  }

  void RigidBodyParticleData::show_tree(std::ostream &out) const {
    for (unsigned int i=0; i< data_.size(); ++i) {
      out << "Node " << i << ": ";
      for (unsigned int j=0; j< data_[i].storage_.size(); ++j) {
        out << data_[i].storage_[j] << " ";
      }
      out << ": " << data_[i].s_ << std::endl;
    }
  }
  void RigidBodyCollisionData::add_data(Particle *p,
                                        const algebra::Sphere3D &s) {
    IMP_assert(data_.find(p) == data_.end(),
               "Already have data for " << p->get_name());
    data_[p]= RigidBodyParticleData();
    data_[p].set_sphere(0, s);
    data_[p].set_leaf(0, std::vector<unsigned int>(1,0));
  }
  void RigidBodyCollisionData::add_data(Particle *p) {
    data_[p]= RigidBodyParticleData();
  }
  RigidBodyParticleData &RigidBodyCollisionData::get_data(Particle *p) {
    IMP_assert(data_.find(p) != data_.end(),
               "Internal error, not set up for particle "
               << p->get_name());
    return data_.find(p)->second;
  }
  const RigidBodyParticleData &
  RigidBodyCollisionData::get_data(Particle *p) const {
    IMP_assert(data_.find(p) != data_.end(),
               "Internal error, not set up for particle "
               << p->get_name());
    return data_.find(p)->second;
  }
  bool RigidBodyCollisionData::has_data(Particle *p) const {
    return data_.find(p) != data_.end();
  }
}

const algebra::Sphere3D
RigidClosePairScore::get_transformed(Particle *a,
                                     const algebra::Sphere3D &s) const {
  if (RigidBodyDecorator::is_instance_of(a)) {
    RigidBodyDecorator d(a);
    return algebra::Sphere3D(d.get_transformation().transform(s.get_center()),
                             s.get_radius());
  } else {
    XYZRDecorator d(a, rk_);
    return d.get_sphere();
  }
}

Particle *RigidClosePairScore::get_member(Particle *a,
                                          unsigned int i) const {
   if (RigidBodyDecorator::is_instance_of(a)) {
     RigidBodyDecorator d(a);
     return d.get_member(i).get_particle();
   } else {
     return a;
   }
}

double RigidClosePairScore::process(Particle *a, Particle *b,
                                  DerivativeAccumulator *dacc) const {
  const internal::RigidBodyParticleData &da= data_.get_data(a);
  const internal::RigidBodyParticleData &db= data_.get_data(b);
  std::vector<std::pair<int, int> > stack;
  stack.push_back(std::make_pair(0,0));
  double ret=0;
  do {
    std::pair<int, int> cur= stack.back();
    stack.pop_back();
    for (unsigned int i=0; i< da.get_number_of_children(cur.first);
         ++i) {
      int ci=da.get_child(cur.first, i);
      algebra::Sphere3D si
        = get_transformed(a, da.get_sphere(ci));
      for (unsigned int j=0;
           j< db.get_number_of_children(cur.second); ++j) {
        int cj=db.get_child(cur.second, j);
        algebra::Sphere3D sj
        = get_transformed(a, db.get_sphere(cj));
        if (distance(si, sj) < threshold_) {
          if (da.get_is_leaf(ci)
              && db.get_is_leaf(cj)) {
            for (unsigned int k=0; k< da.get_number_of_particles(ci); ++k) {
              for (unsigned int l=0; l< db.get_number_of_particles(cj); ++l) {
                ret+= ps_->evaluate(get_member(a, da.get_particle(ci, k)),
                                    get_member(b, db.get_particle(cj, l)),
                                    dacc);

              }
            }
          } else {
            stack.push_back(std::make_pair(ci, cj));
          }
        }
      }
    }
  } while (!stack.empty());
  return ret;
}

void
RigidClosePairScore::setup(const algebra::Sphere3Ds &spheres,
                           unsigned int node_index,
                           const internal::SphereIndexes &leaves,
                           internal::RigidBodyParticleData &data) const {
  // compute sphere for root
  // could make more efficient
  IMP_assert(!leaves.empty(), "Don't call me with no spheres");
  algebra::Sphere3Ds ss(leaves.size());
  for (unsigned int i=0; i< leaves.size(); ++i) {
    ss[i]= spheres[leaves[i]];
  }
  algebra::Sphere3D ec= algebra::enclosing_sphere(ss);
  data.set_sphere(node_index, ec);
  if (leaves.size() <10) {
    data.set_leaf(node_index, leaves);
  } else {
    internal::SpheresSplit ss= divide_spheres(spheres, leaves);
    unsigned int nc= data.add_children(node_index, ss.size());
    for (unsigned int i=0; i< ss.size(); ++i) {
      setup(spheres, nc+i, ss[i], data);
    }
  }
}

/* create a tree in a vector where the stored data is
   - indexes of children
   - bounding sphere (in the rigid body internal frame)

   a node always has children, even it is a leaf (if it is a leaf, the child
   is itself). Encode being a leaf by having a negative last index, that being
   the index into the array of particles.
 */
void RigidClosePairScore::setup(Particle *p) const {
  if (RigidBodyDecorator::is_instance_of(p)) {
    // build spheres on internal coordinates
    RigidBodyDecorator d(p);
    algebra::Sphere3Ds spheres(d.get_number_of_members());
    for (unsigned int i=0; i< d.get_number_of_members(); ++i) {
      double r= XYZRDecorator(d.get_member(i).get_particle(), rk_).get_radius();
      algebra::Vector3D v= d.get_member(i).get_internal_coordinates();
      spheres[i]= algebra::Sphere3D(v, r);
    }
    data_.add_data(p);
    // call internal setup on spheres, 0, all indexes
    internal::SphereIndexes leaves(d.get_number_of_members());
    for (unsigned int i=0; i< leaves.size(); ++i) {
      leaves[i]=i;
    }
    setup(spheres, 0, leaves, data_.get_data(p));
  } else {
    XYZRDecorator d(p, rk_);
    data_.add_data(p, d.get_sphere());
  }
  data_.get_data(p).show_tree(std::cout);
}


RigidClosePairScore::RigidClosePairScore(PairScore *app, double threshold,
                                         FloatKey r): ps_(app),
                                                      rk_(r),
                                                      threshold_(threshold)
{}

double RigidClosePairScore::evaluate(Particle *a, Particle *b,
                                     DerivativeAccumulator *da) const {
#if 1
  if (!data_.has_data(a)) {
    setup(a);
  }
  if (!data_.has_data(b)) {
    setup(b);
  }
  double sum= process(a,b, da);
#else
  Particles psa;
  if (RigidBodyDecorator::is_instance_of(a)) {
    psa= RigidBodyDecorator(a).get_member_particles();
  } else {
    psa.push_back(a);
  }
  Particles psb;
  if (RigidBodyDecorator::is_instance_of(b)) {
    psb= RigidBodyDecorator(b).get_member_particles();
  } else {
    psb.push_back(b);
  }
  double sum=0;
  for (unsigned int i=0; i< psa.size(); ++i) {
    for (unsigned int j=0; j< psb.size(); ++j) {
      sum+=ps_->evaluate(psa[i], psb[j], da);
    }
  }
#endif
  return sum;
}


void RigidClosePairScore::show(std::ostream &out) const {
  out << "RigidClosePairScore:\n";
}

IMPCORE_END_NAMESPACE
