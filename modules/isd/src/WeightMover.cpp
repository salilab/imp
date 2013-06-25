/**
 *  \file WeightMover.cpp  \brief A modifier which perturbs a Weight particle.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/WeightMover.h>
#include <IMP/isd/Weight.h>
#include <IMP/base/random.h>
#include <IMP/algebra/vector_generators.h>
#include <boost/random/uniform_real.hpp>

IMPISD_BEGIN_NAMESPACE


WeightMover::WeightMover(Particle *w, double radius):
  core::MonteCarloMover(w->get_model(), "WeightMover%1%"), radius_(radius)
{
// store decorator. If the particle *w has not been decorated,
// this line throws and exception
 w_ = Weight(w);
// initialize oldweights_
 oldweights_ = w_.get_weights();
}

core::MonteCarloMoverResult WeightMover::do_propose() {
  IMP_OBJECT_LOG;

  // store the old weights in case of reject
  oldweights_ = w_.get_weights();

// Draw weights from a uniform distribution of variables that sum to one
// taken from http://stats.stackexchange.com/questions/14059/
//                     generate-uniformly-distributed-weights-that-sum-to-unity

  // get the old x
  Floats x;
  x.push_back(oldweights_[0]);
  for(unsigned i=1;i<oldweights_.get_dimension()-1;++i){
   x.push_back(oldweights_[i]+x[i-1]);
  }

  // zero vector in D dimension
  algebra::VectorKD zero=algebra::VectorKD(Floats(x.size()));

  // generate random perturbation of old components
  algebra::VectorKD dX
    = algebra::get_random_vector_in(algebra::SphereKD(zero, radius_));

  // add perturbation to x and apply reflective boundaries
  for(unsigned i=0;i<x.size();++i){
   if(x[i]+dX[i]>1.0)       x[i]=2.0-x[i]-dX[i];
   else if(x[i]+dX[i]<0.0)  x[i]=-x[i]-dX[i];
   else                     x[i]+=dX[i];
  }

  // sort the new x here
  std::sort(x.begin(), x.end(), std::less<double>( ) );

  // get the new weights
  algebra::VectorKD newweights =
   algebra::VectorKD(Floats(oldweights_.get_dimension()));


  newweights[0]=x[0];
  for(unsigned i=1;i<newweights.get_dimension()-1;++i){
   newweights[i]=x[i]-x[i-1];
  }
  newweights[newweights.get_dimension()-1]=1.0-x[x.size()-1];

  // set the new weights
  w_.set_weights(newweights);

  return core::MonteCarloMoverResult(
          ParticleIndexes(1, w_.get_particle()->get_index()),1.0);

}

void WeightMover::do_reject() {
  w_.set_weights(oldweights_);
}

kernel::ModelObjectsTemp WeightMover::do_get_inputs() const {
  return kernel::ModelObjectsTemp(1, w_.get_particle());
}

IMPISD_END_NAMESPACE
