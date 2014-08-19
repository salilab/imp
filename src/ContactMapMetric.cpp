/**
 *  \file ContactMapMetric.cpp
 *  \brief Contact Map Metric
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#include <IMP/membrane/ContactMapMetric.h>
#include <IMP/algebra.h>
#include <IMP/statistics.h>
#include <IMP/core/XYZ.h>
#include <algorithm>
#include <vector>
#include <IMP/base/vector_property_map.h>

IMPMEMBRANE_BEGIN_NAMESPACE


ContactMapMetric::ContactMapMetric (Particles ps, double r0, int nn, int mm):
 statistics::Metric("Contact Map Metric %1%"),
 ps_(ps), r0_(r0), nn_(nn), mm_(mm){}

void ContactMapMetric::add_configuration(double weight)
{
// store weight
 weight_.push_back(weight);
 algebra::Vector3Ds coords;
 for(unsigned i=0;i<ps_.size();++i){
  coords.push_back(core::XYZ(ps_[i]).get_coordinates());
 }
// getting contact map
 Floats matrix=get_contact_map(coords);
// and storing it
 matrices_.push_back(matrix);
 return;
}

void ContactMapMetric::add_map(Floats matrix, double weight)
{
 IMP_USAGE_CHECK(matrix.size()==(ps_.size()*(ps_.size()-1)/2),
  "Check dimension of the matrix");
// store weight
 weight_.push_back(weight);
// store map
 matrices_.push_back(matrix);
 return;
}

Floats ContactMapMetric::get_contact_map
 (algebra::Vector3Ds coords) const
{
 Floats matrix;
 for(unsigned i=0;i<coords.size()-1;++i){
  for(unsigned j=i+1;j<coords.size();++j){
   double dist=algebra::get_distance(coords[i],coords[j]);
   if(fabs(dist-r0_)<0.001){matrix.push_back((double)nn_/(double)mm_);}
   else{matrix.push_back((1.0-pow(dist/r0_,nn_))/(1.0-pow(dist/r0_,mm_)));}
  }
 }
 return matrix;
}

double ContactMapMetric::get_rmsd(Floats m0, Floats m1) const
{
 double rmsd=0.0;
 for(unsigned i=0;i<m0.size();++i){
  rmsd += ( m0[i] - m1[i] ) * ( m0[i] - m1[i]);
 }
 return sqrt(rmsd/(double) (m0.size()));
}

double ContactMapMetric::get_distance
 (unsigned int i, unsigned int j) const
{
  return get_rmsd(matrices_[i],matrices_[j]);
}

Float ContactMapMetric::get_weight(unsigned i) const
{
 return weight_[i];
}

Floats ContactMapMetric::get_item(unsigned i) const
{
 return matrices_[i];
}

unsigned int ContactMapMetric::get_number_of_items() const
{
  return matrices_.size();
}


IMPMEMBRANE_END_NAMESPACE
