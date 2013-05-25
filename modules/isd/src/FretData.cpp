/**
 *  \file isd/FretData.cpp
 *  \brief Auxiliary class useful for FRET_R restraint
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/isd/FretData.h>
#include <IMP/macros.h>
#include <IMP/Object.h>
#include <IMP/constants.h>
#include <math.h>
#include <iostream>

IMPISD_BEGIN_NAMESPACE

FretData::FretData(Floats d_term, Floats d_center, Floats d_int,
                   Floats s_grid, Float R0, Float Rmin, Float Rmax,
                   bool do_limit):
  base::Object("Data Structure for FretRestraint %1%")
{
  // store grids
  d_center_ = d_center;
  s_grid_   = s_grid;
  d_term_   = d_term;
  // save quantities useful to move on grids
  nbin_.push_back(d_center_.size());
  nbin_.push_back(s_grid_.size());
  nbin_.push_back(d_term_.size());
  dimension_ = nbin_.size();
  // initialize grids
  init_grids(d_int, R0, Rmin, Rmax, do_limit);
}


// we are flattening arrays using a column-major order
unsigned FretData::get_index(unsigned indices[3]) const {
 unsigned index=indices[dimension_-1];
 for(unsigned i=dimension_-1;i>0;--i){
  index = index * nbin_[i-1] + indices[i-1];
 }
 return index;
}

void FretData::init_grids
     (const Floats& d_grid_int, Float R0, Float Rmin, Float Rmax, bool do_limit)
{
  // grid on distance between termini
  for(unsigned l=0; l<d_term_.size(); ++l){
   // grid on sigma
   for(unsigned i=0; i<s_grid_.size(); ++i){
    // grid on distance between center of GMM
    for(unsigned j=0; j<d_center_.size(); ++j){
     Float marg=0.;
     Float norm=0.;
     unsigned kmin=0;
     unsigned kmax=d_grid_int.size();
     // find boundaries for marginalization
     if(do_limit){
      kmin=get_closest(d_grid_int,std::max(Rmin,d_term_[l]-Rmax));
      kmax=get_closest(d_grid_int,d_term_[l]+Rmax);
     }
     // do the marginalization
     for(unsigned k=kmin+1; k<kmax; ++k){

      Float dx = d_grid_int[k] - d_grid_int[k-1];

      Float prob   = get_probability(d_grid_int[k],   d_center_[j], s_grid_[i]);
      Float probm1 = get_probability(d_grid_int[k-1], d_center_[j], s_grid_[i]);

      Float kernel   = get_kernel( d_grid_int[k],   R0 );
      Float kernelm1 = get_kernel( d_grid_int[k-1], R0 );

      marg += ( kernel * prob + kernelm1 * probm1 ) / 2.0 * dx;
      norm += ( prob + probm1 ) / 2.0 * dx;
     }
     // store in grid_ and norm_
     grid_.push_back(marg);
     norm_.push_back(norm);
    }
   }
  }
}

int FretData::get_closest(std::vector<double> const& vec, double value)
                                                            const {
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

// get probability density
Float FretData::get_probability(Float distn, Float dist, Float sigma) const
{
 Float a = 0.5 * distn / dist / sqrt( 2. * IMP::PI ) / sigma;
 Float b = exp( - ( distn - dist ) * ( distn - dist ) / 2. / sigma / sigma );
 Float c = exp( - ( distn + dist ) * ( distn + dist ) / 2. / sigma / sigma );
 return a * ( b  - c );
}

// get Fret kernel
Float FretData::get_kernel(Float dist, Float R0) const
{
 Float  R0_dist = R0/dist;
 Float  R3 = R0_dist * R0_dist * R0_dist;
 return 1. / ( 1. + R3 * R3 );
}

// get marginal element and normalization
FloatPair FretData::get_marginal_element
                         (Float d_term, Float d_center, Float sigma) const
{
  unsigned j = get_closest(d_center_, d_center);
  unsigned i = get_closest(s_grid_,   sigma);
  unsigned l = get_closest(d_term_,   d_term);
  unsigned indices[3] = {j,i,l};
  unsigned index = get_index(indices);
  return FloatPair(grid_[index],norm_[index]);
}

IMPISD_END_NAMESPACE
