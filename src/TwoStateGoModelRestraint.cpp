/**
 *  \file TwoStateGoModelRestraint.cpp \brief FRET_R restraint
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/membrane/TwoStateGoModelRestraint.h>
#include <numeric>
#include <IMP/core/XYZ.h>
#include <IMP/Particle.h>
#include <string>
#include <iostream>
#include <math.h>
#include <limits>

IMPMEMBRANE_BEGIN_NAMESPACE

TwoStateGoModelRestraint::TwoStateGoModelRestraint
  (Particles ps, Particles psA, Particles psB,
   Float Beta, Float DeltaV, Float Cutoff):
   Restraint(ps[0]->get_model(),"Two-state Go-model Restraint"),
   ps_(ps), Beta_(Beta), DeltaV_(DeltaV) {

   set_parameters(psA, psB, Cutoff);
}

void TwoStateGoModelRestraint::set_parameters
                   (Particles psA, Particles psB, Float cutoff) {

 for(unsigned i=0; i<ps_.size()-1; ++i){
  for(unsigned j=i+1; j<ps_.size(); ++j){

  // check if ps_ are member of the same rigid body
   if(core::RigidMember::get_is_setup(ps_[i]) &&
        core::RigidMember::get_is_setup(ps_[j])){
    if(core::RigidMember(ps_[i]).get_rigid_body() ==
       core::RigidMember(ps_[j]).get_rigid_body()) { continue; }
   }

   // get distance in structure A and B
   Float distA = core::get_distance(core::XYZ(psA[i]),core::XYZ(psA[j]));
   Float distB = core::get_distance(core::XYZ(psB[i]),core::XYZ(psB[j]));

   // if lower than cutoff add to native attractive map
   if(distA < cutoff) { native_attrA_[ IntPair(i,j) ] = distA; }
   if(distB < cutoff) { native_attrB_[ IntPair(i,j) ] = distB; }

  }
 }

}

double TwoStateGoModelRestraint::get_contribution
                                              (Float dist, Float dist0) const {

 return 5.0 * pow(dist0/dist,12) - 6.0 * pow(dist0/dist,10);
}

double TwoStateGoModelRestraint::get_native_potential
 (const std::map< IntPair, Float>& native) const {

 Float score = 0.;

 // map iterator
 std::map< IntPair, Float>::const_iterator it;

 // Cycle on native attractive
 for(it=native.begin(); it!=native.end();++it){
  // get indexes
  IntPair ij  = (*it).first;
  int i = ij.first;
  int j = ij.second;
  // get distance
  Float dist0 = (*it).second;
  Float dist  = core::get_distance(core::XYZ(ps_[i]),core::XYZ(ps_[j]));
  score += std::min(get_contribution(dist,dist0)+1., 1.);
 }

 return score  / static_cast<double>(native.size());
}

double
TwoStateGoModelRestraint::unprotected_evaluate(DerivativeAccumulator *da) const
{
 // check if derivatives are requested
 IMP_USAGE_CHECK(!da, "Derivatives not available");

 // get basin A potential
 Float Va = get_native_potential(native_attrA_);

 // get basin B potential
 Float Vb = get_native_potential(native_attrB_);

 double VMB;

 if( Vb + DeltaV_ > Va ){
  VMB = Beta_ * Va             -log(1.0+exp( -Beta_ * ( Vb + DeltaV_ - Va ) ) );
 } else {
  VMB = Beta_ * (Vb + DeltaV_) -log(1.0+exp( -Beta_ * ( Va - Vb - DeltaV_ ) ) );
 }

 return VMB;
}

kernel::ModelObjectsTemp TwoStateGoModelRestraint::do_get_inputs() const {
  ParticlesTemp ret;
  ret.insert(ret.end(), ps_.begin(), ps_.end());
  return ret;
}
/*
ContainersTemp TwoStateGoModelRestraint::get_input_containers() const {
  return ContainersTemp();
}
*/

IMPMEMBRANE_END_NAMESPACE
