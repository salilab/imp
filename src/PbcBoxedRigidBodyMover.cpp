/**
 *  \file PbcBoxedRigidBodyMover.cpp
 *  \brief A mover that transforms a rigid body
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/membrane/PbcBoxedRigidBodyMover.h>
#include <IMP/core/XYZ.h>
#include <IMP/isd/Scale.h>
#include <IMP/algebra/vector_generators.h>
#include <list>

IMPMEMBRANE_BEGIN_NAMESPACE

PbcBoxedRigidBodyMover::PbcBoxedRigidBodyMover(core::RigidBody d,
                          Particles ps,
                          Float max_translation, Float max_angle,
                          algebra::Vector3Ds centers,
                          algebra::Transformation3Ds transformations,
                          Particle *px, Particle *py, Particle *pz):

  symmetry:RigidBodyMover(d,ps,max_translation,max_angle,
   centers,transformations)
{
// store Scale particles
 px_ = px;
 py_ = py;
 pz_ = pz;
}
/*
Particles PbcBoxedRigidBodyMover::get_particles(Particles ps)
{
 Particles ps_norb;
 for(unsigned i=0;i<ps.size();++i){
  if(!core::RigidMember::get_is_setup(ps[i])){
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
  if(core::RigidMember::get_is_setup(ps[i])){
   core::RigidBody rb = core::RigidMember(ps[i]).get_rigid_body();
   std::vector<core::RigidBody>::iterator it = find(rbs.begin(), rbs.end(), rb);
   if(it==rbs.end()){rbs.push_back(rb);}
  }
 }
 return rbs;
}
*/

algebra::Vector3D PbcBoxedRigidBodyMover::get_vector(algebra::Vector3D center){
 Float sx = isd::Scale(px_).get_scale();
 Float sy = isd::Scale(py_).get_scale();
 Float sz = isd::Scale(pz_).get_scale();
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

core::MonteCarloMoverResult PbcBoxedRigidBodyMover::do_propose() {
  return symmetry::RigidBodyMover::do_propose();
}

void PbcBoxedRigidBodyMover::reset_move() {
/*// reset reference frame of master rigid body
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
*/
  return symmetry::RigidBodyMover::reset_move();

}

ParticlesTemp PbcBoxedRigidBodyMover::do_get_inputs() const {
 ParticlesTemp ret=symmetry::RigidBodyMover::do_get_inputs();
 ret.push_back(px_);
 ret.push_back(py_);
 ret.push_back(pz_);
 return ret;
}

void PbcBoxedRigidBodyMover::show(std::ostream &out) const {
  out << "max translation: " << max_translation_ << "\n";
  out << "max angle: " << max_angle_ << "\n";
}
IMPMEMBRANE_END_NAMESPACE
