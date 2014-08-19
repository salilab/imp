/**
 *  \file ChiSquareMetric.cpp
 *  \brief Contact Map Metric
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#include <IMP/membrane/ChiSquareMetric.h>
#include <IMP/algebra.h>
#include <IMP/statistics.h>
#include <IMP/core/XYZ.h>
#include <algorithm>
#include <vector>
#include <IMP/base/vector_property_map.h>
#include <math.h>


IMPMEMBRANE_BEGIN_NAMESPACE

ChiSquareMetric::ChiSquareMetric(Floats nu_exp, int constr_type):
 statistics::Metric("ChiSquare Metric %1%"), nu_exp_(nu_exp),
 constr_type_(constr_type){}

void ChiSquareMetric::add_configuration(Floats nu, Floats stddev, double weight)
{
// store weight
 weight_.push_back(weight);
// Yield. If the constructor for normal chi2 is used, store non-normalized nu
 if(constr_type_==0){
  nus_.push_back(nu);
  // standard deviation
  stddev_.push_back(stddev);
// else normalize vector (nu-nu_exp)/stddev
 } else {
  double norm2=0.;
  for(unsigned i=0; i<nu.size(); ++i){
   norm2 += pow(( nu[i] - nu_exp_[i] ) / stddev[i],2);
  }
  norm_.push_back(sqrt(norm2));
  for(unsigned i=0; i<nu.size(); ++i){
    nu[i] = ( nu[i] - nu_exp_[i] ) / stddev[i];
  }
  nus_.push_back(nu);
 }
 return;
}

double ChiSquareMetric::get_chisquare_exp(unsigned i) const
{
 double chi2=0.0;
 if(constr_type_==0){
  for(unsigned k=0;k<nu_exp_.size();++k){
   chi2 += ( nus_[i][k] - nu_exp_[k] ) * ( nus_[i][k] - nu_exp_[k] ) /
           stddev_[i][k] / stddev_[i][k] ;
  }
 } else {
  for(unsigned k=0;k<nu_exp_.size();++k){
   chi2 += nus_[i][k] * nus_[i][k];
  }
 }
 return chi2 / (double) (nus_[i].size());
}

double ChiSquareMetric::get_chisquare(unsigned i, unsigned j) const
{
 double chi2=0.0;
 for(unsigned k=0;k<nus_[i].size();++k){
  chi2 += ( nus_[i][k] - nus_[j][k] ) * ( nus_[i][k] - nus_[j][k] ) /
          ( stddev_[i][k] * stddev_[i][k] + stddev_[j][k] * stddev_[j][k] );
 }
 return chi2 / (double) (nus_[i].size());
}

double ChiSquareMetric::get_scalarchisquare(unsigned i, unsigned j) const
{
 double chi2=0.0;
 for(unsigned k=0;k<nus_[i].size();++k){
  chi2 += nus_[i][k] * nus_[j][k] / norm_[i] / norm_[j];
 }
 return acos(chi2);
}

double ChiSquareMetric::get_distance
 (unsigned int i, unsigned int j) const
{
  if(constr_type_==0){
   return get_chisquare(i,j);
  } else {
   return get_scalarchisquare(i,j);
  }
}

Float ChiSquareMetric::get_weight(unsigned i) const
{
 return weight_[i];
}

unsigned int ChiSquareMetric::get_number_of_items() const
{
  return nus_.size();
}

Floats ChiSquareMetric::get_nu(unsigned i) const
{
 return nus_[i];
}

Floats ChiSquareMetric::get_stddev(unsigned i) const
{
 return stddev_[i];
}


IMPMEMBRANE_END_NAMESPACE
