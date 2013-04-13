/**
 *  \file CellMover.cpp
 *  \brief A mover that transforms a rigid body
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/membrane/CellMover.h>
#include <IMP/core/XYZ.h>
#include <IMP/isd2/Scale.h>
#include <IMP/algebra/vector_generators.h>
#include <list>

IMPMEMBRANE_BEGIN_NAMESPACE

CellMover::CellMover(Particle *p, Particles ps, Float max_translation):

  Mover(p->get_model(), "CellMover%1%")
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
  if(!core::RigidMember::particle_is_instance(ps[i])){
   ps_norb.push_back(ps[i]);
  }
 }
 return ps_norb;
}

core::RigidBodies CellMover::get_rigid_bodies(Particles ps)
{
 std::list<core::RigidBody> rbs_list;
 std::list<core::RigidBody>::iterator iit;
 core::RigidBodies rbs;
 for(unsigned i=0;i<ps.size();++i){
  if(core::RigidMember::particle_is_instance(ps[i])){
   rbs_list.push_back(core::RigidMember(ps[i]).get_rigid_body());
  }
 }
 rbs_list.unique();
 for (iit = rbs_list.begin(); iit != rbs_list.end(); iit++){
  rbs.push_back(*iit);
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

ParticlesTemp CellMover::propose_move(Float f) {
  IMP_LOG(VERBOSE,"CellMover::f is  : " << f <<std::endl);
  {
    ::boost::uniform_real<> rand(0,1);
    double fc =rand(base::random_number_generator);
    if (fc > f) return ParticlesTemp();
  }

  boost::uniform_real<> rand(0,1);
  boost::normal_distribution<double> mrng(0, max_translation_);
  boost::variate_generator<RandomNumberGenerator&,
                           boost::normal_distribution<double> >
                          sampler(random_number_generator, mrng);

// scale decorator
  isd2::Scale sp = isd2::Scale(p_);

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

// particle moved
  ParticlesTemp ret;
  ret.insert(ret.end(), ps_.begin(), ps_.end());
  ret.push_back(p_);

  return ret;
}

void CellMover::reset_move() {
// reset scale
 isd2::Scale(p_).set_scale(old_scale_);
// reset positions of particles
 for(unsigned i=0;i<ps_norb_.size();++i){
  core::XYZ(ps_norb_[i]).set_coordinates(oldcoords_[i]);
 }
// reset positions of rigid bodies
 for(unsigned i=0;i<rbs_.size();++i){
  rbs_[i].set_reference_frame(algebra::ReferenceFrame3D(oldtrs_[i]));
 }
}

ParticlesTemp CellMover::get_output_particles() const {
 ParticlesTemp ret;
 ret.insert(ret.end(), ps_.begin(), ps_.end());
 ret.push_back(p_);
 return ret;
}

void CellMover::do_show(std::ostream &out) const {
  out << "max translation: " << max_translation_ << "\n";
}
IMPMEMBRANE_END_NAMESPACE
