/**
 *  \file BoxedMover.cpp
 *  \brief A mover that keeps a particle in a box
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/membrane/BoxedMover.h>
#include <IMP/core.h>
#include <IMP/algebra.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/atom.h>
#include <iostream>

IMPMEMBRANE_BEGIN_NAMESPACE

BoxedMover::BoxedMover(Particle *p,Float max_tr,
                       algebra::Vector3Ds centers):
  MonteCarloMover(p->get_model(), "BoxMover%1%")
{
  IMP_LOG(VERBOSE,"start BoxedMover constructor");
  p_ = p;
  max_tr_ = max_tr;
  centers_ = centers;
  oldcoord_ = core::XYZ(p_).get_coordinates();
  IMP_LOG(VERBOSE,"finish mover construction" << std::endl);
}

MonteCarloMoverResult BoxedMover::do_propose() {
  /*IMP_LOG(VERBOSE,"BoxedMover:: propose move f is  : " << f <<std::endl);
  {
    ::boost::uniform_real<> rand(0,1);
    double fc =rand(random_number_generator);
    if (fc > f) return ParticlesTemp();
  }
  */

   oldcoord_ = core::XYZ(p_).get_coordinates();

   bool outside=true;
   algebra::Vector3D newcoord;

   while(outside){

    algebra::Vector3D tr_x
     = algebra::get_random_vector_in(algebra::Sphere3D
                (algebra::Vector3D(0.0,0.0,0.0),max_tr_));

    newcoord=oldcoord_+tr_x;

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
    if(icell==0) outside=false;
   }

   core::XYZ(p_).set_coordinates(newcoord);

   return MonteCarloMoverResult(kernel::ParticleIndexes(1, p_->get_index())
       , 1.0);
}

void BoxedMover::do_reject() {
 core::XYZ(p_).set_coordinates(oldcoord_);
}

/*
ParticlesTemp BoxedMover::get_output_particles() const {
 ParticlesTemp ret;
 ret.push_back(p_);
 return ret;
}
*/

kernel::ModelObjectsTemp BoxedMover::do_get_inputs() const {
  return kernel::ModelObjectsTemp(1, p);
}

void BoxedMover::show(std::ostream &out) const {
  out << "max translation: " << max_tr_ << "\n";
}

IMPMEMBRANE_END_NAMESPACE
