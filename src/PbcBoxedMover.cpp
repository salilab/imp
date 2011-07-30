/**
 *  \file PbcBoxedMover.cpp
 *  \brief A mover that keeps a particle in a box
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/membrane/PbcBoxedMover.h>
#include <IMP/core.h>
#include <IMP/algebra.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/atom.h>
#include <iostream>

IMPMEMBRANE_BEGIN_NAMESPACE

PbcBoxedMover::PbcBoxedMover(Particle *p, Float max_tr,
                             algebra::Vector3Ds centers,
                             algebra::Transformation3Ds transformations)
{
  IMP_LOG(VERBOSE,"start PbcBoxedMover constructor");
  p_ = p;
  max_tr_ = max_tr;
  centers_ = centers;
  ps_ = atom::Hierarchy(p).get_parent().get_leaves();
  transformations_ = transformations;
  IMP_LOG(VERBOSE,"finish mover construction" << std::endl);
}

void PbcBoxedMover::propose_move(Float f) {
  IMP_LOG(VERBOSE,"PbcBoxedMover:: propose move f is  : " << f <<std::endl);
  {
    ::boost::uniform_real<> rand(0,1);
    double fc =rand(random_number_generator);
    if (fc > f) return;
  }

   algebra::Vector3D tr_x
     = algebra::get_random_vector_in(algebra::Sphere3D
                (algebra::Vector3D(0.0,0.0,0.0),max_tr_));

   algebra::Vector3D newcoord=core::XYZ(p_).get_coordinates()+tr_x;

// find cell
   double mindist=1.0e+24;
   unsigned icell=0;
   for(unsigned int i=0;i<centers_.size();++i){
    double dist=algebra::get_l2_norm(newcoord-centers_[i]);
    if(dist<mindist){
     mindist=dist;
     icell=i;
    }
   }

   algebra::Transformation3D trans=transformations_[icell].get_inverse();

   oldcoords_.clear();
   for(unsigned int i=0;i<ps_.size();++i){
    oldcoords_.push_back(core::XYZ(ps_[i]).get_coordinates());
    algebra::Vector3D trr_x=algebra::Vector3D(0.0,0.0,0.0);
    if(ps_[i]==p_) trr_x=tr_x;
    newcoord=trans.get_transformed(oldcoords_[i]+trr_x);
    core::XYZ(ps_[i]).set_coordinates(newcoord);
   }

}

void PbcBoxedMover::reset_move() {
 for(unsigned int i=0;i<ps_.size();++i){
    core::XYZ(ps_[i]).set_coordinates(oldcoords_[i]);
 }
}

void PbcBoxedMover::do_show(std::ostream &out) const {
  out << "max translation: " << max_tr_ << "\n";
}

IMPMEMBRANE_END_NAMESPACE
