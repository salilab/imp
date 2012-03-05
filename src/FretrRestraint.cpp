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
#include <functional>
#include <algorithm>

IMPMEMBRANE_BEGIN_NAMESPACE

FretrRestraint::FretrRestraint(Particles pd, Particles pa,
                               double R0, double gamma, double Ida,
                               double Pbleach0, double Pbleach1,
                               double fretr, double kappa,
                               std::string name, double multi):
  Restraint("FRET_R Restraint " + name) {

 pd_       = pd;
 pa_       = pa;
 R0_       = R0;
 gamma_    = gamma;
 Ida_      = Ida;
 fretr_    = fretr;
 kappa_    = kappa;
 multi_    = multi;
 nclose_   = std::min((int)pa_.size(),4);
// photobleaching?
 if(Pbleach0 >= 1.0 && Pbleach1 >= 1.0){
  photobleach_ = false;
  Pbleach0_    = 1.0;
 }else{
  photobleach_ = true;
  Pbleach0_    = Pbleach0;
  set_photobleach(Pbleach0,Pbleach1);
 }
}

std::vector<unsigned>
 FretrRestraint::get_state(unsigned index) const {
 std::vector<unsigned> indices;
 unsigned kk=index;
 indices.push_back(index%2);
 for(unsigned int i=1;i<nclose_-1;++i){
  kk=(kk-indices[i-1])/2;
  indices.push_back(kk%2);
 }
 if(nclose_>=2){
  indices.push_back((kk-indices[nclose_-2])/2);
 }
 return indices;
}

double FretrRestraint::get_weight
 (std::vector<unsigned> state, double Pbleach) const
{
 double weight=1.0;
 for(unsigned i=0; i<state.size(); ++i){
  weight*=pow(1.0-Pbleach,1-state[i])*pow(Pbleach,state[i]);
 }
 return weight;
}

void FretrRestraint::set_photobleach(double Pbleach0, double Pbleach1)
{
  states_.clear();
  weight0_.clear();
  weight1_.clear();
  for(unsigned i=0;i<pow(2,nclose_);++i){
   std::vector<unsigned> state=get_state(i);
   states_.push_back(state);
   weight0_.push_back(get_weight(state,Pbleach0));
   weight1_.push_back(get_weight(state,Pbleach1));
  }
}

double
FretrRestraint::unprotected_evaluate(DerivativeAccumulator *da) const
{
// check if derivatives are requested
// IMP_USAGE_CHECK(!da, "Derivatives not available");

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

 double sumFi_0 = 0.0;
 double sumFi_1 = 0.0;
 for(unsigned i = 0; i < pd_.size(); ++i){
  Floats power6(pa_.size());
  for(unsigned j = 0; j < pa_.size(); ++j){
   double p  = R0_/core::get_distance(core::XYZ(pd_[i]),core::XYZ(pa_[j]));
   power6[j] = p*p*p*p*p*p;
  }
// sort power6 from large to small
  std::sort(power6.begin(), power6.end(), std::greater<double>( ) );
// states_ contains all the possible photobleaching states of nclose_ particles
  for(unsigned j = 0; j < states_.size(); ++j){
   double Fi = 0.0;
// only the closest nclose_ pairs contributes (if alive) to Fi
   for(unsigned k = 0; k < states_[j].size(); ++k){
    Fi += (double)states_[j][k] * power6[k];
   }
   double inverseFi = 1.0 / ( 1.0 + Fi );
// each photobleaching state must be properly weighted
   sumFi_0 += inverseFi * weight0_[j];
   sumFi_1 += inverseFi * weight1_[j];
  }
 }

 double fretr = ( Ida_ * sumFi_0 + (double) pa_.size() / multi_ * Pbleach0_ +
                  gamma_ * ( (double) pd_.size() - sumFi_0 ) ) /
                ( Ida_ * sumFi_1 + (double) pa_.size() / multi_ ) - fretr_;

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
                ( Ida_ * sumFi + (double) pa_.size() / multi_ ) - fretr_;

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
