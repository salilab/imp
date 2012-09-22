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
#include <IMP/compatibility/vector_property_map.h>

IMPMEMBRANE_BEGIN_NAMESPACE


ChiSquareMetric::ChiSquareMetric():
 statistics::Metric("ChiSquare Metric %1%"){}

void ChiSquareMetric::add_configuration(Floats nu, Floats stddev, double weight)
{
// store weight
 weight_.push_back(weight);
// Yield
 nus_.push_back(nu);
// standard deviation
 stddev_.push_back(stddev);
 return;
}

double ChiSquareMetric::get_chisquare(Floats nu0, Floats stddev0,
                            Floats nu1, Floats stddev1) const
{
 double chi2=0.0;
 for(unsigned i=0;i<nu0.size();++i){
  chi2 += ( nu0[i] - nu1[i] ) * ( nu0[i] - nu1[i] ) /
          ( stddev0[i] * stddev0[i] + stddev1[i] * stddev1[i] );
 }
 return chi2 / (double) (nu0.size());
}

double ChiSquareMetric::get_distance
 (unsigned int i, unsigned int j) const
{
  return get_chisquare(nus_[i], stddev_[i], nus_[j], stddev_[j]);
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

void ChiSquareMetric::do_show(std::ostream &) const {
}

IMPMEMBRANE_END_NAMESPACE
