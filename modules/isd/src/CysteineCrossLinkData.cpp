/**
 *  \file CysteineCrossLinkData.cpp
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/isd/CysteineCrossLinkData.h>
#include <IMP/macros.h>
#include <IMP/base/Object.h>
#include <IMP/constants.h>
#include <math.h>
#include <boost/math/special_functions/erf.hpp>
#include <boost/math/special_functions/fpclassify.hpp>

IMPISD_BEGIN_NAMESPACE



CysteineCrossLinkData::CysteineCrossLinkData(
        double fexp, Floats fmod_grid, Floats omega_grid, Floats omega0_grid,
        int prior_type) :
  base::Object("Data Structure for CysteineCrossLinkRestraint %1%")
{
 prior_type_ = prior_type;
//fexp is the experimental frequency
//fmod is the model frequency
//omega0 is the typical value for omega, i.e., the experimental uncertainty

//this constructor calculates the marginal likelihood using a
//truncated gaussian function
//to account for outliers

// Store omega0 grid
 omega0_grid_ = omega0_grid;

// Store the fmod grid
 fmod_grid_ = fmod_grid;

// Store omega grid
 omega_grid_ = omega_grid;

 fexp_=fexp;

 for(unsigned k=0;k<omega0_grid_.size();++k){

  double omega0 = omega0_grid_[k];

  Floats grid;

 for(unsigned i=0;i<fmod_grid_.size();++i){

  double fmod = fmod_grid_[i];

  double cumul = 0;
  for(unsigned j=1;j<omega_grid_.size();++j){

   double omj=omega_grid_[j];
   double omjm1=omega_grid_[j-1];
   double dom = omj - omjm1;

   double priorj=get_omega_prior(omj,omega0);
   double priorjm1=get_omega_prior(omjm1,omega0);

   double pj=get_element(fexp_,fmod,omj)*priorj;
   double pjm1=get_element(fexp_,fmod,omjm1)*priorjm1;
   cumul+= (pj+pjm1)/2.0*dom;

  }
  grid.push_back(cumul);

 }
 grid_.push_back(grid);
}

}




Floats CysteineCrossLinkData::get_omegas(Floats fmods, double omega0) const
{
 Floats omegas;
 for(unsigned n=0;n<fmods.size();++n){

  double cumul = 0;
  double cumul2 = 0;

  for(unsigned j=1;j<omega_grid_.size();++j){
     double omj=omega_grid_[j];
     double omjm1=omega_grid_[j-1];
     double dom = omj - omjm1;

     double priorj=get_omega_prior(omj,omega0);
     double priorjm1=get_omega_prior(omjm1,omega0);

     double pj=get_element(fexp_,fmods[n],omj)*priorj;
     double pjm1=get_element(fexp_,fmods[n],omjm1)*priorjm1;

     double pj2=get_element(fexp_,fmods[n],omj)*priorj*omj;
     double pjm12=get_element(fexp_,fmods[n],omjm1)*priorjm1*omjm1;

     cumul+= (pj+pjm1)/2.0*dom;
     cumul2+= (pj2+pjm12)/2.0*dom;
 }

 omegas.push_back(cumul2/cumul);
 }
 return omegas;
}


double CysteineCrossLinkData::get_omega(double fmod, double omega0) const
{
  double cumul = 0;
  double cumul2 = 0;

  unsigned is = get_closest(omega0_grid_,omega0);
  double omega0i = omega0_grid_[is];


  for(unsigned j=1;j<omega_grid_.size();++j){
     double omj=omega_grid_[j];
     double omjm1=omega_grid_[j-1];
     double dom = omj - omjm1;

     double priorj=get_omega_prior(omj,omega0i);
     double priorjm1=get_omega_prior(omjm1,omega0i);

     double pj=get_element(fexp_,fmod,omj)*priorj;
     double pjm1=get_element(fexp_,fmod,omjm1)*priorjm1;

     double pj2=get_element(fexp_,fmod,omj)*priorj*omj;
     double pjm12=get_element(fexp_,fmod,omjm1)*priorjm1*omjm1;

     cumul+= (pj+pjm1)/2.0*dom;
     cumul2+= (pj2+pjm12)/2.0*dom;
 }

 double omega=cumul2/cumul;
 return omega;
}


double CysteineCrossLinkData::get_standard_deviation(double fmod,
                                                          double omega0) const
{
 //this method evaluate the standard deviation of the marginal
 //likelihood distribution given an
 //expected value fmod and a typical error omega0
 //DONT USE at place of the get_standard error() class defined
 //in the CysteineCrossLinkRestraint


 // evaluate the probability density for the observed frequencies
 Floats f_density;

 for(unsigned i=0;i<fmod_grid_.size();++i){

  double f = fmod_grid_[i];

  double cumul = 0;
  for(unsigned j=1;j<omega_grid_.size();++j){

   double omj=omega_grid_[j];
   double omjm1=omega_grid_[j-1];
   double dom = omj - omjm1;

   double priorj=get_omega_prior(omj,omega0);
   double priorjm1=get_omega_prior(omjm1,omega0);

   double pj=get_element(f,fmod,omj)*priorj;
   double pjm1=get_element(f,fmod,omjm1)*priorjm1;
   cumul+= (pj+pjm1)/2.0*dom;

  }

  f_density.push_back(cumul);
 }

  //evaluate the average
  double norm = 0;
  double cumul = 0;
  for(unsigned j=1;j<fmod_grid_.size();++j){
     double fj=fmod_grid_[j];
     double fjm1=fmod_grid_[j-1];
     double df = fj - fjm1;

     double pj=f_density[j];
     double pjm1=f_density[j-1];

     double pj2=pj*fj;
     double pjm12=pjm1*fjm1;

     norm+= (pj+pjm1)/2.0*df;
     cumul+= (pj2+pjm12)/2.0*df;
 }

 double f_ave=cumul/norm;

 //evaluate the standard deviation
  cumul = 0;
  for(unsigned j=1;j<fmod_grid_.size();++j){
     double fj=fmod_grid_[j];
     double fjm1=fmod_grid_[j-1];
     double df = fj - fjm1;

     double pj2=f_density[j]*(fj-f_ave)*(fj-f_ave);
     double pjm12=f_density[j-1]*(fjm1-f_ave)*(fjm1-f_ave);

     cumul+= (pj2+pjm12)/2.0*df;
 }
 double f_stand_dev=sqrt(cumul/norm);
 return f_stand_dev;
}


double CysteineCrossLinkData::get_omega_prior(double omega, double omega0) const
{
   double prior=0.0;
   if (prior_type_==0){
     //jeffrey's prior
     prior=1.0/omega;
   }

   if (prior_type_==1){
     //that's nice, the tails are similar, therefore it is independent of omega0
     //it is a gamma distribution first degree. Omega is overestimated
     prior=1.0/omega0*exp(-omega/omega0);
   }

   if (prior_type_==2){
     //this prior on omega is a gamma distribution second degree
     prior=4.0*omega/omega0/omega0*exp(-2.0*omega/omega0);
   }

   if (prior_type_==3){
     //cauchy-style outlier, as described in Sivia's book
     prior=2.0*omega0/sqrt(IMP::PI)/omega/omega*exp(-omega0*omega0/omega/omega);
   }
   return prior;
}



Floats CysteineCrossLinkData::get_nonmarginal_elements(double fexp,
                                  Floats fmods, double omega) const {
     Floats probs;

     for (unsigned n=0;n<fmods.size();n++){
            probs.push_back(get_element(fexp,fmods[n], omega));
        }
     return probs;
}


double CysteineCrossLinkData::get_nonmarginal_element(double fexp,
                                   double fmod, double omega) const {
     double prob=get_element(fexp,fmod, omega);
     return prob;
}


Floats CysteineCrossLinkData::get_marginal_elements(Floats fmods, double omega0)
                                                             const {
    Floats probs;
    unsigned is = get_closest(omega0_grid_,omega0);
    for (unsigned n=0;n<fmods.size();n++){

      unsigned id = get_closest(fmod_grid_,fmods[n]);
      probs.push_back(grid_[is][id]);
    }
    return probs;
}

double CysteineCrossLinkData::get_marginal_element(double fmod, double omega0)
                                                      const {
    unsigned is = get_closest(omega0_grid_,omega0);
    unsigned id = get_closest(fmod_grid_,fmod);
    double  prob=grid_[is][id];
    return prob;
}

int CysteineCrossLinkData::get_closest(std::vector<double> const& vec,
                                                double value) const {

  std::vector<double>::const_iterator ub =
    std::lower_bound(vec.begin(), vec.end(), value);
  if ( ub == vec.end() ){
   return vec.size()-1;
  }else if( ub == vec.begin() ){
   return 0;
  }else{
   std::vector<double>::const_iterator lb = ub - 1;
   int index = int( lb - vec.begin() );
   if( fabs( *ub - value ) < fabs( *lb - value ) ){
    index = int( ub - vec.begin() );
   }
   return index;
  }
}

double CysteineCrossLinkData::get_element
 (double fexp, double fmod, double omega) const
{
 double inv_omega = 1.0 / ( omega * sqrt(2.0) );

 double prob = inv_omega / sqrt( IMP::PI ) *
 exp ( - ( fmod - fexp ) * ( fmod - fexp ) * inv_omega * inv_omega );


 double norm = 0.5 * ( boost::math::erf(   fmod         * inv_omega ) -
                       boost::math::erf( ( fmod - 1.0 ) * inv_omega ) );

 return prob/norm;
}


IMPISD_END_NAMESPACE
