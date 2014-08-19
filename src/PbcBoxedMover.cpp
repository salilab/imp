/**
 *  \file PbcBoxedMover.cpp
 *  \brief A mover that keeps a particle in a box
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/membrane/PbcBoxedMover.h>
#include <IMP/core.h>
#include <IMP/isd/Scale.h>
#include <IMP/algebra.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/atom.h>
#include <iostream>

IMPMEMBRANE_BEGIN_NAMESPACE

PbcBoxedMover::PbcBoxedMover(Particle *p, Particles ps, Float max_tr,
                             algebra::Vector3Ds centers,
                             algebra::Transformation3Ds transformations,
                             Particle *px, Particle *py, Particle *pz):
  symmetry::BallMover(p,ps,max_tr,centers,transformations)
{ /* These are in base class
  p_ = p;
  max_tr_ = max_tr;
  centers_ = centers;
  ps_ = ps;
  transformations_ = transformations;
  */
  // store Scale particles
  px_ = px;
  py_ = py;
  pz_ = pz;
}

algebra::Vector3D PbcBoxedMover::get_vector(algebra::Vector3D center){
 Float sx = isd::Scale(px_).get_scale();
 Float sy = isd::Scale(py_).get_scale();
 Float sz = isd::Scale(pz_).get_scale();
 algebra::Vector3D newcenter = algebra::Vector3D(center[0]*sx,
                                                 center[1]*sy,
                                                 center[2]*sz);
 return newcenter;
}

algebra::Transformation3D PbcBoxedMover::get_transformation
 (algebra::Transformation3D trans){
 algebra::Rotation3D rr=trans.get_rotation();
 algebra::Vector3D   tt=trans.get_translation();
 algebra::Transformation3D newtrans(rr, get_vector(tt));
 return newtrans;
}

core::MonteCarloMoverResult PbcBoxedMover::do_propose() {
   return symmetry::BallMover::do_propose();
}

/*
ParticlesTemp PbcBoxedMover::get_output_particles() const {
 ParticlesTemp ret;
 ret.insert(ret.end(), ps_.begin(), ps_.end());
 ret.push_back(px_);
 ret.push_back(py_);
 ret.push_back(pz_);
 return ret;
}
*/

ModelObjectsTemp PbcBoxedMover::do_get_inputs() const {
  kernel::ParticlesTemp ret=symmetry::BallMover::do_get_inputs();
  ret.push_back(px_);
  ret.push_back(py_);
  ret.push_back(pz_);

  return ret;
}

void PbcBoxedMover::do_reject() {
 /*for(unsigned int i=0;i<ps_.size();++i){
    core::XYZ(ps_[i]).set_coordinates(oldcoords_[i]);
 } */
  symmetry::BallMover::do_reject();
}

void PbcBoxedMover::show(std::ostream &out) const {
  out << "max translation: " << max_tr_ << "\n";
}

IMPMEMBRANE_END_NAMESPACE
