/**
 *  \file isd/FretRestraint.cpp
 *  \brief FRET_R restraint
 *  to use in vivo FRET data [Muller et al. Mol Biol Cell 16, 3341, 2005]
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <stdio.h>
#include <IMP/constants.h>
#include <IMP/isd/FretRestraint.h>
#include <IMP/isd/FretData.h>
#include <IMP/core/XYZ.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/isd/Scale.h>
#include <math.h>
#include <limits.h>
#include <IMP/base/random.h>
#include <iostream>
#include <IMP/algebra/ReferenceFrame3D.h>
#include "IMP/core/rigid_bodies.h"

IMPISD_BEGIN_NAMESPACE

// First constructor, with particles for multiple donors and acceptors
// Bayesian parameters are: kda, Ida, R0, uncertainty,
// and photobleaching survival rate
FretRestraint::FretRestraint
  (kernel::Particles pd, kernel::Particles pa,
   kernel::Particle *kda, kernel::Particle *Ida, kernel::Particle *R0, kernel::Particle *sigma0,
   kernel::Particle *Pbl, double fexp, double m_d, double m_a):
   pd_(pd), pa_(pa), kda_(kda), Ida_(Ida),
   R0_(R0), sigma0_(sigma0), Pbl_(Pbl), fexp_(fexp),
   multi_d_(m_d), constr_type_(0) {

// number of acceptors
   Na_ = static_cast<unsigned>(floor(static_cast<double>(pa_.size())/m_a+0.5));
// build list of state vectors (on-the-fly average over photobleached states)
   for(unsigned i = 0; i < (1<<Na_); ++i){
    // get state vector
    std::vector<unsigned> indices=get_indices(i, Na_);
    // and add to list
    states_.push_back(indices);
   }
};


// Second constructor, used when feeding ensamble averages of Forster factor
// Bayesian parameters: kda, Ida, sumFi, uncertainty,
// and photobleaching survival rate
FretRestraint::FretRestraint
  (kernel::Particle *kda, kernel::Particle *Ida, kernel::Particle *sumFi,
   kernel::Particle *sigma0, kernel::Particle *Pbl, double fexp):
   kda_(kda), Ida_(Ida), sumFi_(sumFi), sigma0_(sigma0),
   Pbl_(Pbl), fexp_(fexp), constr_type_(1) {};

// Third constructor, with model for linker noise, only single molecule for now
// Bayesian parameters: kda, Ida, uncertainty,
// and photobleaching survival rate
FretRestraint::FretRestraint
  (kernel::Particle *prd, algebra::Vector3D GMMterd,
   algebra::Vector3Ds GMMctrd, Floats GMMwd, Floats GMMsigd,
   kernel::Particle *pra, algebra::Vector3D GMMtera,
   algebra::Vector3Ds GMMctra, Floats GMMwa, Floats GMMsiga,
   kernel::Particle *kda, kernel::Particle *Ida, kernel::Particle *sigma0,
   kernel::Particle *Pbl, FretData *data, double fexp):
   prd_(prd), GMMterd_(GMMterd), GMMctrd_(GMMctrd),
   pra_(pra), GMMtera_(GMMtera), GMMctra_(GMMctra),
   kda_(kda), Ida_(Ida), sigma0_(sigma0), Pbl_(Pbl),
   data_(data), fexp_(fexp), constr_type_(2) {

// store sigma and weight
   for (unsigned id=0; id<GMMctrd_.size(); id++){
    for (unsigned ia=0; ia<GMMctra_.size(); ia++){
     GMMsig_.push_back(sqrt(GMMsigd[id]*GMMsigd[id]+GMMsiga[ia]*GMMsiga[ia]));
     GMMw_.push_back(GMMwd[id]*GMMwa[ia]);
    }
   }
}

// reset experimental value
void FretRestraint::set_experimental_value(double fexp) {
 fexp_ = fexp;
}

// get average sigma
double FretRestraint::get_average_sigma(double fmod) const
{
  double sigma0 = Scale(sigma0_).get_scale();

  double B = 0.5 * log(fexp_/fmod) * log(fexp_/fmod) + sigma0 * sigma0;

  double sigmaave = sqrt( IMP::PI * B );

  return sigmaave;
}

// lognormal standard error
double FretRestraint::get_standard_error() const
{
 double fretr = get_model_fretr();

 double sigma = get_average_sigma(fretr);

 double expsigma2 = exp(sigma*sigma);

 double var = (expsigma2-1.)*fretr*fretr*expsigma2;

 return sqrt(var);
}

double FretRestraint::get_model_fretr() const
{
  if(constr_type_==0) { return get_model_fretr_type_0(); }
  else                { return get_model_fretr_type_1(); }
}

// we are flattening arrays using a column-major order
std::vector<unsigned> FretRestraint::get_indices
 (unsigned index, int dimension) const {
 std::vector<unsigned> indices;
 unsigned kk = index;
 indices.push_back( index % 2 );
 for(int i = 1; i < dimension-1; ++i){
  kk = ( kk - indices[i-1] ) / 2;
  indices.push_back( kk % 2 );
 }
 if(dimension >= 2){
  indices.push_back( ( kk - indices[dimension-2] ) / 2 );
 }
 return indices;
}

double FretRestraint::get_sumFi(double Pbleach) const
{
 // a minimum of 3 acceptors is always considered
 const unsigned Nmin = 3;
 // or power6_.size() if lower than Nmin
 unsigned Na = std::min(static_cast<unsigned>(power6_.size()), Nmin);
 // if power6_ is empty, return 1.0/(1.0+epsilon)
 if( Na == 0 ){return 1.0/(1.0+std::numeric_limits<double>::epsilon());}
 // Beyond that, thresold at 10% of the greatest
 double thres = power6_[0] * 0.1;
 // increase Na by adding the number of
 // entries in power6_ above the thresold value
 for(unsigned i = Nmin; i < power6_.size(); ++i){
  if( power6_[i] < thres ){ break; }
  Na = i + 1;
 }
 // calculate ensemble average
 double sumFi = 0.;
 // enumerate all possible photobleached states
 for(unsigned i = 0; i < (1<<Na); ++i){
  double Fi = 0.;
  double weight = 1.;
  for(unsigned j = 0; j < Na; ++j){
   // 1 is alive, 0 is dead
   double w = static_cast<double>(states_[i][j]);
   Fi += power6_[j] * w;
   // building probability of the state
   weight *=  ( w * Pbleach + ( 1. - w ) * ( 1. - Pbleach ) );
  }
  sumFi += 1.0 / ( 1.0 + Fi ) * weight;
 }
 return sumFi;
}

// type 0
double FretRestraint::get_model_fretr_type_0() const
{
// thresold for forster factor
 const double thres = 0.01;
// get scales
 double Ida = Scale(Ida_).get_scale();
 double kda = Scale(kda_).get_scale();
 double R0  = Scale(R0_).get_scale();
 double Pbl = Scale(Pbl_).get_scale();

 double sumFi    = 0.;
 double sumFi_bl = 0.;

 for(unsigned i = 0; i < pd_.size(); ++i){
   power6_.clear();
   for(unsigned j = 0 ; j < pa_.size(); ++j){
    double dist =
     std::max(core::get_distance(core::XYZ(pd_[i]),core::XYZ(pa_[j])),
              std::numeric_limits<double>::epsilon( ));
    double R  = R0 / dist;
    double R6 = R * R * R * R * R * R;
    if(R6 > thres){ power6_.push_back(R6); }
   }
   // reorder forster factors in descending order
   std::sort(power6_.begin(), power6_.end(), std::greater<double>( ) );
   // sum at most Na_ forster factors
   if( power6_.size() > Na_ ){ power6_.resize(Na_); }
   double Fi = std::accumulate(power6_.begin(), power6_.end(), 0.);
   // when no acceptors are bleached
   sumFi += 1.0 / ( 1.0 + Fi ) / multi_d_;
   if(Pbl >= 0.97){
    sumFi_bl += 1.0 / ( 1.0 + Fi ) / multi_d_;
   // when acceptors are bleatched
   } else {
    sumFi_bl += get_sumFi(Pbl) / multi_d_;
   }
 }

 double Na = static_cast<double>(Na_);
 double Nd = static_cast<double>(pd_.size()) / multi_d_;

 double fretr = ( Ida * sumFi    + Na + kda * ( Nd - sumFi ) ) /
                ( Ida * sumFi_bl + Na );

 return fretr;
}

algebra::Vector3Ds FretRestraint::get_current_centers
                      (kernel::Particle *p, const algebra::Vector3Ds& ctrs) const {

 algebra::Vector3Ds new_ctrs;
 for(unsigned i=0; i<ctrs.size(); ++i){
     new_ctrs.push_back(get_current_center(p,ctrs[i]));
 }
 return new_ctrs;
}

algebra::Vector3D FretRestraint::get_current_center
                      (kernel::Particle *p, const algebra::Vector3D& ctr) const {

 algebra::ReferenceFrame3D rf=core::RigidBody(p).get_reference_frame();
 return rf.get_global_coordinates(ctr);
}

double FretRestraint::get_sumFi() const
{
  // get distance between termini
  algebra::Vector3D terd=get_current_center(prd_, GMMterd_);
  algebra::Vector3D tera=get_current_center(pra_, GMMtera_);
  Float d_term = algebra::get_distance(terd, tera);
  // get current global coordinates of the GMM centers for donor
  algebra::Vector3Ds ctrd=get_current_centers(prd_, GMMctrd_);
  // and for acceptor
  algebra::Vector3Ds ctra=get_current_centers(pra_, GMMctra_);
  // calculate sumFi
  double num=0.; double den=0.;
  for (unsigned id=0; id<ctrd.size(); id++){
   for (unsigned ia=0; ia<ctra.size(); ia++){
    Float d_center = algebra::get_distance(ctrd[id],ctra[ia]);
    unsigned index = id * ctra.size() + ia;
    FloatPair marg_norm=
                    data_->get_marginal_element(d_term,d_center,GMMsig_[index]);
    num += marg_norm.first  * GMMw_[index];
    den += marg_norm.second * GMMw_[index];
   }
  }
  return num / den;
}

double FretRestraint::get_model_fretr_type_1() const
{
  double sumFi;
  if(constr_type_ == 1) sumFi = Scale(sumFi_).get_scale();
  if(constr_type_ == 2) sumFi = get_sumFi();
  double Ida = Scale(Ida_).get_scale();
  double kda = Scale(kda_).get_scale();
  double Pbl = Scale(Pbl_).get_scale();

  double fretr = ( Ida * sumFi + 1. + kda * ( 1. - sumFi ) ) /
                 ( Ida * ( Pbl * sumFi + ( 1. - Pbl ) ) + 1. );
  return fretr;
}

/* call for probability */
// Probability is the marginalization of a log-normal distribution
// multiplied by a Cauchy-like prior for sigma
double FretRestraint::get_probability() const
{

 double fretr = get_model_fretr();

 double log_eps = log(fexp_/fretr);

 double sigma0 = Scale(sigma0_).get_scale();

 double prob = sqrt(2.) * sigma0 / fexp_ / IMP::PI /
               ( log_eps * log_eps + 2. * sigma0 * sigma0 );

 return prob;
}

double FretRestraint::
                 unprotected_evaluate(DerivativeAccumulator *accum) const
{
 double score;

 double prob = get_probability();

//check if probability is too low (e.g. equal to zero)
//and assign its value to the smallest double
 if( prob <= std::numeric_limits<double>::epsilon( ) ){
      prob = std::numeric_limits<double>::epsilon( );
 }

 score = -log(prob);

 if (accum)
 {
 }

 return score;
}

/* Return all particles whose attributes are read by the restraints. To
   do this, ask the pair score what particles it uses.*/
ModelObjectsTemp  FretRestraint::do_get_inputs() const
{
  kernel::ParticlesTemp ret;
  ret.push_back(Ida_);
  ret.push_back(kda_);
  ret.push_back(sigma0_);
  ret.push_back(Pbl_);
  if(constr_type_==0){
   ret.insert(ret.end(), pd_.begin(), pd_.end());
   ret.insert(ret.end(), pa_.begin(), pa_.end());
   ret.push_back(R0_);
  }
  if(constr_type_==1){ ret.push_back(sumFi_); }
  if(constr_type_==2){
   ret.push_back(prd_);
   ret.push_back(pra_);
  }
  return ret;
}

IMPISD_END_NAMESPACE
