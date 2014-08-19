/**
 *  \file RMSDMetric.cpp
 *  \brief Distance RMSD Metric with PBC and identical particles
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#include <IMP/membrane/RMSDMetric.h>
#include <IMP/algebra.h>
#include <IMP/statistics.h>
#include <IMP/core/XYZ.h>
#include <algorithm>
#include <vector>
#include <IMP/base/vector_property_map.h>

IMPMEMBRANE_BEGIN_NAMESPACE


RMSDMetric::RMSDMetric (Particles ps):
  statistics::Metric("Distance RMSD Metric %1%"), ps_(ps){}

void RMSDMetric::add_configuration(double weight)
{
 weight_.push_back(weight);
 algebra::Vector3Ds coords;
 for(unsigned i=0;i<ps_.size();++i){
  coords.push_back(core::XYZ(ps_[i]).get_coordinates());
 }
 coords_.push_back(coords);
 return;
}

double RMSDMetric::get_rmsd
 (algebra::Vector3Ds v0, algebra::Vector3Ds v1) const
{
 double rmsd=0.0;
 int    np=ps_.size();
 for(int i=0;i<np;++i){
  double dist=algebra::get_distance(v0[i], v1[i]);
  rmsd += dist*dist;
 }
 return sqrt(rmsd/(double)np);
}

double RMSDMetric::get_distance
 (unsigned int i, unsigned int j) const
{
  return get_rmsd(coords_[i],coords_[j]);
}

Float RMSDMetric::get_weight(unsigned i)
{
 return weight_[i];
}

unsigned int RMSDMetric::get_number_of_items() const {
  return coords_.size();
}


IMPMEMBRANE_END_NAMESPACE
