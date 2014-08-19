/**
 *  \file CellMover.cpp
 *  \brief A mover that transforms a rigid body
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/membrane/CellMover.h>
#include <IMP/core/XYZ.h>
#include <IMP/isd/Scale.h>
#include <IMP/algebra/vector_generators.h>
#include <list>

IMPMEMBRANE_BEGIN_NAMESPACE

CellMover::CellMover(Particle *p,Particles ps, Float max_translation):

  MonteCarloMover(p->get_model(), "CellMover%1%")
{
// store Scale particle
 p_ = p;
// list of all particles
 ps_ = ps;
// list of particles not belonging to rigid bodies
 ps_norb_ = get_particles(ps);
// list of rigid bodies
 rbs_ = get_rigid_bodies(ps);
// other stuff
 max_translation_= max_translation;
}

Particles CellMover::get_particles(Particles ps)
{
 Particles ps_norb;
 for(unsigned i=0;i<ps.size();++i){
  if(!core::RigidMember::get_is_setup(ps[i])){
   ps_norb.push_back(ps[i]);
  }
 }
 return ps_norb;
}

std::vector<core::RigidBody> CellMover::get_rigid_bodies(Particles ps)
{
 std::vector<core::RigidBody> rbs;
 for(unsigned i=0;i<ps.size();++i){
  if(core::RigidMember::get_is_setup(ps[i])){
   core::RigidBody rb = core::RigidMember(ps[i]).get_rigid_body();
   std::vector<core::RigidBody>::iterator it = find(rbs.begin(),
                          rbs.end(), rb);
   if(it==rbs.end()){rbs.push_back(rb);}
  }
 }
 return rbs;
}

algebra::Vector3D CellMover::get_transformed(Float cf, algebra::Vector3D oc)
{
 Float a1 = oc[0] +      oc[1] / sqrt(3.0);
 Float a2 =         2. * oc[1] / sqrt(3.0);
 Float x = cf * ( a1 - 0.5 * a2 );
 Float y = cf * ( sqrt(3.0) / 2. * a2 );
 Float z = oc[2];
 algebra::Vector3D newcoord=algebra::Vector3D(x,y,z);
 return newcoord;
}

core::MonteCarloMoverResult CellMover::do_propose() {
  /*IMP_LOG(VERBOSE,"CellMover::f is  : " << f <<std::endl);
  {
    ::boost::uniform_real<> rand(0,1);
    double fc =rand(base::random_number_generator);
    if (fc > f) return ParticlesTemp();
  }
  */
  boost::uniform_real<> rand(0,1);
  boost::normal_distribution<double> mrng(0, max_translation_);
  boost::variate_generator<base::RandomNumberGenerator&,
                           boost::normal_distribution<double>>
                          sampler(base::random_number_generator, mrng);

// scale decorator
  isd::Scale sp = isd::Scale(p_);

// store old scale
  old_scale_ = sp.get_scale();

// set new scale
  sp.set_scale(old_scale_ + sampler());

// define compression factor
  Float cf = sp.get_scale() / old_scale_;

// compress particle positions
  oldcoords_.clear();
  for(unsigned i=0;i<ps_norb_.size();++i){
   core::XYZ xyz = core::XYZ(ps_norb_[i]);
   algebra::Vector3D oc = xyz.get_coordinates();
   oldcoords_.push_back(oc);
   xyz.set_coordinates(get_transformed(cf,oc));
  }

// and the rigid bodies
  oldtrs_.clear();
  for(unsigned i=0;i<rbs_.size();++i){
    algebra::Transformation3D ot =
     rbs_[i].get_reference_frame().get_transformation_to();
    oldtrs_.push_back(ot);
    algebra::Rotation3D rr = ot.get_rotation();
    algebra::Vector3D   oc = ot.get_translation();
    algebra::Vector3D   nc = get_transformed(cf,oc);
    algebra::Transformation3D t3d(rr, nc);
    rbs_[i].set_reference_frame(algebra::ReferenceFrame3D(t3d));
  }
//
  ParticleIndexes ret;
  ret.push_back(p_->get_index());
  for(unsigned i=0;i<ps_.size();++i) {
      ret.push_back(ps_[i]->get_index());
  }

  return core::MonteCarloMoverResult(ret, 1.0);
}

void CellMover::do_reject() {
// reset scale
 isd::Scale(p_).set_scale(old_scale_);
// reset positions of particles
 for(unsigned i=0;i<ps_norb_.size();++i){
  core::XYZ(ps_norb_[i]).set_coordinates(oldcoords_[i]);
 }
// reset positions of rigid bodies
 for(unsigned i=0;i<rbs_.size();++i){
  rbs_[i].set_reference_frame(algebra::ReferenceFrame3D(oldtrs_[i]));
 }
}

kernel::ModelObjectsTemp CellMover::do_get_inputs() const {

  kernel::ModelObjectsTemp ret;
  ret.push_back(p_);
  ret.insert(ret.end(),ps_.begin(),ps_.end());

  return ret;
}

void CellMover::show(std::ostream &out) const {
  out << "max translation: " << max_translation_ << "\n";
}
IMPMEMBRANE_END_NAMESPACE
