/**
 *  \file FretrRestraint.cpp \brief FRET_R restraint
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/membrane/FretrRestraint.h>
#include <IMP/core/XYZ.h>
#include <IMP/Particle.h>

IMPMEMBRANE_BEGIN_NAMESPACE

FretrRestraint::FretrRestraint(Particles pd, Particles pa,
                               double R0, double gamma, double Ida,
                               double Pbleach0, double Pbleach1,
                               double fretr, double kappa,
                               std::string name, double multi):
 Restraint("FRET_R Restraint " + name), pd_(pd), pa_(pa),
 R0_(R0), gamma_(gamma), Ida_(Ida), fretr_(fretr), kappa_(kappa),
 Pbleach0_(Pbleach0), Pbleach1_(Pbleach1), mcsteps_(1000)
{
// adjust length
 power6_.resize(pa_.size());
// number of acceptors
 Na_ = (unsigned)(floor((double)pa_.size()/multi+0.5));
// set photobleaching
 if(Pbleach0_ >= 1.0 && Pbleach1_ >= 1.0){
  photobleach_ = false;
 }else{
  photobleach_ = true;
  states0_ = get_states(Pbleach0_);
  states1_ = get_states(Pbleach1_);
 }
}

Floats FretrRestraint::get_states(double Pb)
{
 Floats states;
 ::boost::uniform_real<> rand(0.0,1.0);
 for(unsigned i = 0; i < mcsteps_; ++i){
  for(unsigned j = 0; j < Na_; ++j){
   double fc = rand(random_number_generator);
   if( fc < Pb ){states.push_back(1.0);}
   else{states.push_back(0.0);}
  }
 }
 return states;
}

double FretrRestraint::get_sumFi
 (const Floats& power6, const Floats& states) const
{
  double sumFi = 0.;
  double sumFiave;
  double sumFiaveold = -1.0;
  const unsigned iblock = 10;
  const double thres = 0.005;
  for(unsigned i = 0; i < mcsteps_; ++i){
   double Fi = 0.;
   for(unsigned j = 0; j < Na_; ++j){
    Fi += states[ i * Na_ + j ] * power6[j];
   }
   sumFi += 1.0 / ( 1.0 + Fi );
   if( i%iblock == 0 ){
    sumFiave = sumFi / (double)(i+1);
    if( fabs( sumFiave - sumFiaveold ) < thres ){return sumFiave;}
    sumFiaveold = sumFiave;
   }
  }
  return sumFiaveold;
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
  for(unsigned j = 0; j < pa_.size(); ++j){
   double p  = R0_/core::get_distance(core::XYZ(pd_[i]),core::XYZ(pa_[j]));
   power6_[j] = p*p*p*p*p*p;
  }
  std::sort(power6_.begin(), power6_.end(), std::greater<double>( ) );
  sumFi_0 += get_sumFi(power6_,states0_);
  sumFi_1 += get_sumFi(power6_,states1_);
 }

 double fretr = ( Ida_ * sumFi_0 + (double) Na_ +
                  gamma_ * ( (double) pd_.size() - sumFi_0 ) / Pbleach0_ ) /
                ( Ida_ * sumFi_1 + (double) Na_ / Pbleach0_ ) - fretr_;

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
                ( Ida_ * sumFi + (double) Na_ ) - fretr_;

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
