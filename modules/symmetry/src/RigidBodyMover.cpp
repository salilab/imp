/**
 *  \file RigidBodyMover.cpp
 *  \brief A mover that keeps a rigid body in a box
 *
 *  Copyright 2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/symmetry/RigidBodyMover.h>
#include <IMP/core/XYZ.h>
#include <IMP/algebra/vector_generators.h>
#include <list>

IMPSYMMETRY_BEGIN_NAMESPACE

RigidBodyMover::RigidBodyMover(core::RigidBody d, Particles ps,
                        Float max_tr, Float max_ang,
                        algebra::Vector3Ds ctrs,
                        algebra::Transformation3Ds trs):
  MonteCarloMover(d->get_model(), "RigidBodyMover%1%")
{
 // master rigid body
 d_ = d;
 // list of slave particles
 ps_ = ps;
 // list of slave particles not member of rigid bodies
 ps_norb_ = get_particles(ps);
 // list of slave rigid bodies
 rbs_ = get_rigid_bodies(ps);
 // maximum translation
 max_tr_= max_tr;
 // maximum rotation
 max_ang_ = max_ang;
 // list of all cell centers
 ctrs_ = ctrs;
 // list of transformation from primitive to all cells
 trs_ = trs;
}

Particles RigidBodyMover::get_particles(Particles ps)
{
 Particles ps_norb;
 for(unsigned i=0;i<ps.size();++i){
  if(!core::RigidMember::get_is_setup(ps[i])){
   ps_norb.push_back(ps[i]);
  }
 }
 return ps_norb;
}

std::vector<core::RigidBody> RigidBodyMover::get_rigid_bodies (Particles ps)
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

core::MonteCarloMoverResult RigidBodyMover::do_propose() {
  IMP_OBJECT_LOG;

  // store last reference frame of master rigid body
  oldtr_= d_.get_reference_frame().get_transformation_to();

  // generate new coordinates of center of mass of master rb
  algebra::Vector3D nc =
   algebra::get_random_vector_in(algebra::Sphere3D(d_.get_coordinates(),
                                                   max_tr_));

  // find center of the closest cell
  double mindist=1.0e+24;
  unsigned icell=0;
  for(unsigned int i=0;i<ctrs_.size();++i){
   // calculate distance between nc and cell center
   double dist=algebra::get_l2_norm(nc-ctrs_[i]);
   // find minimum distance
   if( dist < mindist ){ mindist=dist; icell=i; }
  }

  // find inverse transformation
  algebra::Transformation3D cell_tr = trs_[icell].get_inverse();

  // r: rotation around random axis
  algebra::VectorD<3> axis =
    algebra::get_random_vector_on(algebra::Sphere3D(algebra::VectorD<3>(0.0,
                                                                        0.0,
                                                                        0.0),
                                                    1.));

  ::boost::uniform_real<> rand(-max_ang_,max_ang_);
  Float angle =rand(base::random_number_generator);
  algebra::Rotation3D r
    = algebra::get_rotation_about_axis(axis, angle);

  // ri: composing rotation of reference frame transformation and
  // rotation due to boundary-crossing
  algebra::Rotation3D ri=cell_tr.get_rotation()*
     d_.get_reference_frame().get_transformation_to().get_rotation();
  // rc: composing ri with random rotation r
  algebra::Rotation3D rc = r*ri;

  // new reference frame for master rb
  d_.set_reference_frame(algebra::ReferenceFrame3D(
                   algebra::Transformation3D(rc, cell_tr.get_transformed(nc))));


  // set new coordinates for slave particles
  oldcoords_.clear();
  for(unsigned i=0;i<ps_norb_.size();++i){
   core::XYZ xyz = core::XYZ(ps_norb_[i]);
   algebra::Vector3D oc = xyz.get_coordinates();
   // store old coordinates
   oldcoords_.push_back(oc);
   // apply cell transformation
   algebra::Vector3D nc=cell_tr.get_transformed(oc);
   xyz.set_coordinates(nc);
  }

  // set new reference frames for slave rbs
  oldtrs_.clear();
  for(unsigned i=0;i<rbs_.size();++i){
    algebra::Transformation3D ot =
     rbs_[i].get_reference_frame().get_transformation_to();
    // store old reference frame transformation
    oldtrs_.push_back(ot);
    // create new reference frame
    algebra::Rotation3D rr=cell_tr.get_rotation()*ot.get_rotation();
    algebra::Vector3D   tt=cell_tr.get_transformed(rbs_[i].get_coordinates());
    // set new reference frame for slave rbs
    rbs_[i].set_reference_frame(algebra::ReferenceFrame3D(
                                             algebra::Transformation3D(rr,tt)));
  }

  ParticlesTemp ret=ParticlesTemp(1, d_);
  ret.insert(ret.end(), ps_.begin(), ps_.end());

  return core::MonteCarloMoverResult(kernel::get_indexes(ret), 1.0);
}

void RigidBodyMover::do_reject() {
 // reset reference frame of master rb
 d_.set_reference_frame(algebra::ReferenceFrame3D(oldtr_));
 oldtr_ = algebra::Transformation3D();
 // reset positions of slave particles
 for(unsigned i=0;i<ps_norb_.size();++i){
  core::XYZ(ps_norb_[i]).set_coordinates(oldcoords_[i]);
 }
 // reset reference frame of slave rbs
 for(unsigned i=0;i<rbs_.size();++i){
  rbs_[i].set_reference_frame(algebra::ReferenceFrame3D(oldtrs_[i]));
 }
}

ModelObjectsTemp RigidBodyMover::do_get_inputs() const {
 ParticlesTemp ret=ParticlesTemp(1, d_);
 ret.insert(ret.end(), ps_.begin(), ps_.end());
 return ret;
}

IMPSYMMETRY_END_NAMESPACE
