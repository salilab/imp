/**
 *  \file PbcBoxedRigidBodyMover.cpp
 *  \brief A mover that transforms a rigid body
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/membrane/PbcBoxedRigidBodyMover.h>
#include <IMP/core/XYZ.h>
#include <IMP/isd2/Scale.h>
#include <IMP/algebra/vector_generators.h>
#include <list>

IMPMEMBRANE_BEGIN_NAMESPACE

PbcBoxedRigidBodyMover::PbcBoxedRigidBodyMover(core::RigidBody d,
                          Particles ps,
                          Float max_translation, Float max_angle,
                          algebra::Vector3Ds centers,
                          algebra::Transformation3Ds transformations,
                          Particle *px, Particle *py, Particle *pz):

  Mover(d->get_model(), "PbcBoxedRigidBodyMover%1%")
{
// master rigid body
 d_ = d;
// list of all slave particles
 ps_ = ps;
// list of slave particles not belonging to rigid bodies
 ps_norb_ = get_particles(ps);
// list of slave rigid bodies
 rbs_ = get_rigid_bodies(ps);
// other stuff
 max_translation_= max_translation;
 max_angle_ = max_angle;
 centers_ = centers;
 transformations_ = transformations;
// store Scale particles
 px_ = px;
 py_ = py;
 pz_ = pz;
}

Particles PbcBoxedRigidBodyMover::get_particles(Particles ps)
{
 Particles ps_norb;
 for(unsigned i=0;i<ps.size();++i){
  if(!core::RigidMember::particle_is_instance(ps[i])){
   ps_norb.push_back(ps[i]);
  }
 }
 return ps_norb;
}

std::vector<core::RigidBody>
 PbcBoxedRigidBodyMover::get_rigid_bodies (Particles ps)
{
 std::vector<core::RigidBody> rbs;
 for(unsigned i=0;i<ps.size();++i){
  if(core::RigidMember::particle_is_instance(ps[i])){
   core::RigidBody rb = core::RigidMember(ps[i]).get_rigid_body();
   std::vector<core::RigidBody>::iterator it = find(rbs.begin(), rbs.end(), rb);
   if(it==rbs.end()){rbs.push_back(rb);}
  }
 }
 return rbs;
}

algebra::Vector3D PbcBoxedRigidBodyMover::get_vector(algebra::Vector3D center){
 Float sx = isd2::Scale(px_).get_scale();
 Float sy = isd2::Scale(py_).get_scale();
 Float sz = isd2::Scale(pz_).get_scale();
 algebra::Vector3D newcenter = algebra::Vector3D(center[0]*sx,
                                                 center[1]*sy,
                                                 center[2]*sz);
 return newcenter;
}

algebra::Transformation3D PbcBoxedRigidBodyMover::get_transformation
 (algebra::Transformation3D trans){
 algebra::Rotation3D rr=trans.get_rotation();
 algebra::Vector3D   tt=trans.get_translation();
 algebra::Transformation3D newtrans(rr, get_vector(tt));
 return newtrans;
}

ParticlesTemp PbcBoxedRigidBodyMover::propose_move(Float f) {
  IMP_LOG(VERBOSE,"PbcBoxedRigidBodyMover::f is  : " << f <<std::endl);
  {
    ::boost::uniform_real<> rand(0,1);
    double fc =rand(base::random_number_generator);
    if (fc > f) return ParticlesTemp();
  }

// store last reference frame of master rigid body
  last_transformation_= d_.get_reference_frame().get_transformation_to();

// generate random translation
  algebra::VectorD<3> translation
    = algebra::get_random_vector_in(algebra::Sphere3D(d_.get_coordinates(),
                                                      max_translation_));
// find closest cell center
  double mindist=1.0e+24;
  unsigned icell=0;
  for(unsigned int i=0;i<centers_.size();++i){
   double dist=algebra::get_l2_norm(translation-get_vector(centers_[i]));
   if(dist<mindist){
    mindist=dist;
    icell=i;
   }
  }

// adjusting translation to boundary-crossing
  algebra::Transformation3D trans=
   get_transformation(transformations_[icell]).get_inverse();
  translation=trans.get_transformed(translation);

// generate rotation around random axis
  algebra::VectorD<3> axis =
    algebra::get_random_vector_on(algebra::Sphere3D(algebra::VectorD<3>(0.0,
                                                                        0.0,
                                                                        0.0),
                                                    1.));
  ::boost::uniform_real<> rand(-max_angle_,max_angle_);
  Float angle =rand(base::random_number_generator);
  algebra::Rotation3D r
    = algebra::get_rotation_about_axis(axis, angle);

// ri: composing rotation of reference frame transformation and
// rotation due to boundary-crossing
  algebra::Rotation3D ri=trans.get_rotation()*
     d_.get_reference_frame().get_transformation_to().get_rotation();
// rc: composing random rotation with ri
  algebra::Rotation3D rc = r*ri;

// final transformation
  algebra::Transformation3D t(rc, translation);
  IMP_LOG(VERBOSE,"PbcBoxedRigidBodyMover:: propose move : " << t << std::endl);
  d_.set_reference_frame(algebra::ReferenceFrame3D(t));

// move the slave particles with trans
  oldcoords_.clear();
  for(unsigned i=0;i<ps_norb_.size();++i){
   core::XYZ xyz = core::XYZ(ps_norb_[i]);
   algebra::Vector3D oc = xyz.get_coordinates();
   oldcoords_.push_back(oc);
   algebra::Vector3D newcoord=trans.get_transformed(oc);
   xyz.set_coordinates(newcoord);
  }
// and the slave rigid bodies
  oldtrs_.clear();
  for(unsigned i=0;i<rbs_.size();++i){
    algebra::Transformation3D ot =
     rbs_[i].get_reference_frame().get_transformation_to();
    oldtrs_.push_back(ot);
    algebra::Rotation3D rr=trans.get_rotation()*ot.get_rotation();
    algebra::VectorD<3> tt=trans.get_transformed(rbs_[i].get_coordinates());
    algebra::Transformation3D t3d(rr, tt);
    rbs_[i].set_reference_frame(algebra::ReferenceFrame3D(t3d));
  }

// particle moved
  ParticlesTemp ret=ParticlesTemp(1, d_);
  if(icell!=0){ret.insert(ret.end(), ps_.begin(), ps_.end());}

  return ret;
}

void PbcBoxedRigidBodyMover::reset_move() {
// reset reference frame of master rigid body
 d_.set_reference_frame(algebra::ReferenceFrame3D(last_transformation_));
 last_transformation_ = algebra::Transformation3D();
// reset positions of slave particles
 for(unsigned i=0;i<ps_norb_.size();++i){
  core::XYZ(ps_norb_[i]).set_coordinates(oldcoords_[i]);
 }
// reset positions of slave rigid bodies
 for(unsigned i=0;i<rbs_.size();++i){
  rbs_[i].set_reference_frame(algebra::ReferenceFrame3D(oldtrs_[i]));
 }
}

ParticlesTemp PbcBoxedRigidBodyMover::get_output_particles() const {
 ParticlesTemp ret=ParticlesTemp(1, d_);
 ret.insert(ret.end(), ps_.begin(), ps_.end());
 ret.push_back(px_);
 ret.push_back(py_);
 ret.push_back(pz_);
 return ret;
}

void PbcBoxedRigidBodyMover::do_show(std::ostream &out) const {
  out << "max translation: " << max_translation_ << "\n";
  out << "max angle: " << max_angle_ << "\n";
}
IMPMEMBRANE_END_NAMESPACE
