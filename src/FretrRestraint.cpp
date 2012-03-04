/**
 *  \file FretrRestraint.cpp \brief FRET_R restraint
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/membrane/FretrRestraint.h>
#include <numeric>
#include <IMP/core/XYZ.h>
#include <IMP/Particle.h>
#include <iostream>

IMPMEMBRANE_BEGIN_NAMESPACE

FretrRestraint::FretrRestraint(Particles pd, Particles pa,
                               double R0, double gamma, double Ida,
                               double Pbleach0, double Pbleach1,
                               double fretr, double kappa,
                               std::string name):
  Restraint("FRET_R Restraint " + name) {

 pd_     = pd;
 pa_     = pa;
 R0_     = R0;
 gamma_  = gamma;
 Ida_    = Ida;
 fretr_  = fretr;
 kappa_  = kappa;
// photobleaching?
 if(Pbleach0 >= 1.0 && Pbleach1 >= 1.0){
  photobleach_ = false;
 }else{
  photobleach_ = true;
  mcsteps_ = 100;
  set_photobleach(Pbleach0,Pbleach1);
 }
}

void FretrRestraint::set_photobleach(double Pbleach0, double Pbleach1)
{
  ::boost::uniform_real<> rand(0.0,1.0);
  bleach0_.clear();
  bleach1_.clear();
  for(unsigned imc=0;imc<mcsteps_;++imc){
   for(unsigned j = 0; j < pa_.size(); ++j){
    double fc0 =rand(random_number_generator);
    if(fc0 < Pbleach0){bleach0_.push_back(1.0);}
    else{bleach0_.push_back(0.0);}
    double fc1 =rand(random_number_generator);
    if(fc1 < Pbleach1){bleach1_.push_back(1.0);}
    else{bleach1_.push_back(0.0);}
   }
  }
}

double
FretrRestraint::unprotected_evaluate(DerivativeAccumulator *da) const
{
// check if derivatives are requested
 IMP_USAGE_CHECK(!da, "Derivatives not available");

 double fretr;
 if(photobleach_){
   fretr = get_bleach_fretr();
 }else{
   fretr = get_nobleach_fretr();
 };

 return 0.5 * kappa_ * fretr * fretr;
}

double FretrRestraint::get_bleach_fretr() const
{
 // pre-calculate power6
 Floats power6(pd_.size()*pa_.size());
 for(unsigned i = 0; i < pd_.size(); ++i){
  for(unsigned j = 0; j < pa_.size(); ++j){
   double p = R0_/core::get_distance(core::XYZ(pd_[i]),core::XYZ(pa_[j]));
   power6[ i * pa_.size() + j ] = p*p*p*p*p*p;
  }
 }

 double fretr = 0.0;

 for(unsigned imc=0;imc<mcsteps_;++imc){

// number of acceptors alive when measuring FRET channel
  double Na = 0.0;
  for(unsigned j = 0; j < pa_.size(); ++j){
   Na += bleach0_[ imc * pa_.size() + j ];
  }

  double sumFi_0 = 0.0;
  double sumFi_1 = 0.0;
  for(unsigned i = 0; i < pd_.size(); ++i){
   double Fi_0 = 0.0;
   double Fi_1 = 0.0;
   for(unsigned j = 0; j < pa_.size(); ++j){
    double p6 = power6[ i * pa_.size() + j ];
    Fi_0 += bleach0_[ imc * pa_.size() + j ] * p6;
    Fi_1 += bleach1_[ imc * pa_.size() + j ] * p6;
   }
   sumFi_0 += 1.0 / ( 1.0 + Fi_0 );
   sumFi_1 += 1.0 / ( 1.0 + Fi_1 );
  }

  fretr += ( Ida_*sumFi_0 + Na/7.0 + gamma_*((double)pd_.size()-sumFi_0) ) /
           ( Ida_*sumFi_1 + (double)pa_.size()/7.0 ) - fretr_;
 }

 fretr /= (double) mcsteps_;

 return fretr;
}

double FretrRestraint::get_nobleach_fretr() const
{

 double sumFi = 0.0;
 for(unsigned i = 0; i < pd_.size(); ++i){
  double Fi = 0.0;
  for(unsigned j = 0; j < pa_.size(); ++j){
   double p = R0_/core::get_distance(core::XYZ(pd_[i]),core::XYZ(pa_[j]));
   Fi += p*p*p*p*p*p;
  }
  sumFi += 1.0 / ( 1.0 + Fi );
 }

 double fretr = 1.0 + gamma_ * ( (double) pd_.size() - sumFi ) /
                ( Ida_ * sumFi + (double) pa_.size() / 7.0 ) - fretr_;

 return fretr;
}

ParticlesTemp FretrRestraint::get_input_particles() const {
  ParticlesTemp ret;
  ret.insert(ret.end(), pd_.begin(), pd_.end());
  ret.insert(ret.end(), pa_.begin(), pa_.end());
  return ret;
}

ContainersTemp FretrRestraint::get_input_containers() const {
  return ContainersTemp();
}

void FretrRestraint::do_show(std::ostream &) const
{
}

IMPMEMBRANE_END_NAMESPACE
