/**
 *  \file RigidBodyGridStates.cpp
 *  \brief RigidBody states on a grid
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#include <IMP/membrane/RigidBodyGridStates.h>
#include <IMP/algebra.h>
#include <IMP/core/XYZ.h>
#include <IMP/core/rigid_bodies.h>
#include <IMP/domino/particle_states.h>


IMPMEMBRANE_BEGIN_NAMESPACE

RigidBodyGridStates::RigidBodyGridStates
(algebra::Vector6D begin,   algebra::Vector6D end,
 algebra::Vector6D binsize, Float rot0)
{
  begin_=begin;
  end_=end;
  binsize_=binsize;
  rot0_=rot0;
  nstates_ = 1;
  for(int i=0;i<6;++i){
   nbin_[i]=int((end_[i]-begin_[i])/binsize_[i])+1;
   nstates_ *= nbin_[i];
  }
}

Ints RigidBodyGridStates::get_index(unsigned int index) const
{
 int i[6];
 i[0] = index % nbin_[0];
 i[1] = (index - i[0])/nbin_[0] % nbin_[1];
 i[2] = ((index - i[0])/nbin_[0]-i[1])/nbin_[1] % nbin_[2];
 i[3] = (((index - i[0])/nbin_[0]-i[1])/nbin_[1] -i[2])/nbin_[2] % nbin_[3];
 i[4] = ((((index - i[0])/nbin_[0]-i[1])/nbin_[1] -i[2])/nbin_[2] -i[3])
 /nbin_[3] % nbin_[4];
 i[5] = (((((index - i[0])/nbin_[0]-i[1])/nbin_[1] -i[2])/nbin_[2] -i[3])
 /nbin_[3] -i[4])/nbin_[4];
 return Ints(i, i+6);
}

algebra::ReferenceFrame3D
 RigidBodyGridStates::get_reference_frame(unsigned int i) const {
IMP_USAGE_CHECK(i < nstates_, "Out of range");

 Ints nindex = get_index(i);
 algebra::Rotation3D rot0=
 algebra::get_rotation_about_axis(algebra::Vector3D(0,1,0), rot0_);
 algebra::Rotation3D rotz=
 algebra::get_rotation_about_axis(algebra::Vector3D(0,0,1),
 double(nindex[3])*binsize_[3]+begin_[3]);
 algebra::Rotation3D tilt=
 algebra::get_rotation_about_axis(algebra::Vector3D(0,1,0),
 double(nindex[4])*binsize_[4]+begin_[4]);
 algebra::Rotation3D swing=
 algebra::get_rotation_about_axis(algebra::Vector3D(0,0,1),
 double(nindex[5])*binsize_[5]+begin_[5]);
 algebra::Vector3D trans=
 algebra::Vector3D(double(nindex[0])*binsize_[0]+begin_[0],
 double(nindex[1])*binsize_[1]+begin_[1],
 double(nindex[2])*binsize_[2]+begin_[2]);
 algebra::ReferenceFrame3D state= algebra::ReferenceFrame3D
 (algebra::Transformation3D(algebra::compose(swing,
  algebra::compose(tilt,algebra::compose(rotz,rot0))),trans));
  return state;
}

unsigned int RigidBodyGridStates::get_number_of_particle_states() const {
  return nstates_;
}

void RigidBodyGridStates::load_particle_state(unsigned int i, Particle *p)
 const {
  IMP_USAGE_CHECK(i < nstates_, "Out of range " << i);
  core::RigidBody(p).set_reference_frame
  (RigidBodyGridStates::get_reference_frame(i));
}

void RigidBodyGridStates::show(std::ostream &out) const {
  out << "size: " << nstates_ << std::endl;
}

IMPMEMBRANE_END_NAMESPACE
