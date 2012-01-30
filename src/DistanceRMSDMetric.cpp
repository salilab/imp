/**
 *  \file DistanceRMSDMetric.cpp
 *  \brief Distance RMSD Metric with PBC and identical particles
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#include <IMP/membrane/DistanceRMSDMetric.h>
#include <IMP/algebra.h>
#include <IMP/statistics.h>
#include <IMP/statistics/internal/TrivialPartitionalClustering.h>
#include <algorithm>
#include <vector>
#include <IMP/compatibility/vector_property_map.h>

IMPMEMBRANE_BEGIN_NAMESPACE


DistanceRMSDMetric::DistanceRMSDMetric
 (Particles ps, Ints align, algebra::Transformation3Ds tr):
  statistics::Metric("Distance RMSD Metric %1%"), ps_(ps), tr_(tr)
{
 IMP_USAGE_CHECK(ps.size()==align.size(),
  "Check number of particles and alignment");
 initialize(align);
}

void DistanceRMSDMetric::initialize(Ints align)
{
// create vector of unique types
  Ints types=align;
  std::sort(types.begin(), types.end());
  types.erase(std::unique(types.begin(), types.end()), types.end());

// list of pair types
  std::vector< std::pair<int,int> > pair_types;
  for(unsigned i=0;i<types.size();++i){
   for(unsigned j=i;j<types.size();++j){
    pair_types.push_back(std::pair<int,int>(types[i],types[j]));
   }
  }

// list of particles pairs
  std::vector< std::pair<int,int> > pairs;
  for(unsigned i=0;i<align.size()-1;++i){
   for(unsigned j=i+1;j<align.size();++j){
    pairs.push_back(std::pair<int,int>(align[i],align[j]));
   }
  }

// list of lists of equivalent entries in a distance matrix
  for(unsigned i=0;i<pair_types.size();++i){
   int type0=pair_types[i].first;
   int type1=pair_types[i].second;
   std::vector<unsigned> list;
   for(unsigned j=0;j<pairs.size();++j){
    int type00=pairs[j].first;
    int type11=pairs[j].second;
    if((type0==type00 && type1==type11) || (type0==type11 && type1==type00)){
     list.push_back(j);
    }
   }
   if(list.size()>0){matrixmap_.push_back(list);}
  }
 return;
}

void DistanceRMSDMetric::add_configuration(double weight)
{
// store weight
 weight_.push_back(weight);
// calculate distance matrix
 algebra::Vector3Ds coords;
 for(unsigned i=0;i<ps_.size();++i){
  coords.push_back(core::XYZ(ps_[i]).get_coordinates());
 }
 Floats matrix=get_distance_matrix(coords);
 matrices_.push_back(matrix);
 return;
}

Floats DistanceRMSDMetric::get_distance_matrix
 (algebra::Vector3Ds coords) const
{
 Floats matrix;
 for(unsigned i=0;i<coords.size()-1;++i){
  for(unsigned j=i+1;j<coords.size();++j){
   matrix.push_back(get_distance(coords[i],coords[j]));
  }
 }
 return matrix;
}

Float DistanceRMSDMetric::get_distance
 (algebra::Vector3D v0, algebra::Vector3D v1) const
{
 Floats dists;
 for(unsigned i=0;i<tr_.size();++i){
  dists.push_back(algebra::get_distance(v0,tr_[i].get_transformed(v1)));
 }
 Float mindist=*(min_element (dists.begin(), dists.end()));
 return mindist;
}

double DistanceRMSDMetric::get_drmsd(Floats m0, Floats m1) const
{
 double drmsd=0.0;
 for(unsigned i=0;i<matrixmap_.size();++i){
  Floats dist0;
  Floats dist1;
  for(unsigned j=0;j<matrixmap_[i].size();++j){
   unsigned index=matrixmap_[i][j];
   dist0.push_back(m0[index]);
   dist1.push_back(m1[index]);
  }
  std::sort(dist0.begin(), dist0.end());
  std::sort(dist1.begin(), dist1.end());
  for(unsigned j=0;j<dist0.size();++j){
   drmsd+=( dist0[j] - dist1[j] ) *( dist0[j] - dist1[j] );
  }
 }
 return sqrt(drmsd/(double) (m0.size()));
}

double DistanceRMSDMetric::get_distance
 (unsigned int i, unsigned int j) const
{
  return get_drmsd(matrices_[i],matrices_[j]);
}

Float DistanceRMSDMetric::get_weight(unsigned i)
{
 return weight_[i];
}

unsigned int DistanceRMSDMetric::get_number_of_items() const {
  return matrices_.size();
}

void DistanceRMSDMetric::do_show(std::ostream &) const {
}

statistics::PartitionalClustering* create_gromos_clustering
 (membrane::DistanceRMSDMetric *d, double cutoff)
 {
  compatibility::checked_vector<Ints> clusters;
  unsigned nitems=d->get_number_of_items();

// create vector of neighbors and weights
  std::vector<Ints> neighbors(nitems);
  Floats weights(nitems);
  for(unsigned i=0;i<nitems;++i){
   neighbors[i].push_back((int)i);
   weights[i]=d->get_weight(i);
  }
  for(unsigned i=0;i<nitems-1;++i){
   for(unsigned j=i+1;j<nitems;++j){
    if(d->get_distance(i,j)<cutoff){
     neighbors[i].push_back((int)j);
     weights[i]+=d->get_weight(j);
     neighbors[j].push_back((int)i);
     weights[j]+=d->get_weight(i);
    }
   }
  }

  double maxweight=1.0;
  while(maxweight>0.0)
  {
// find the conf with maximum weight
   maxweight=-1.0;
   unsigned icenter;
   for(unsigned i=0;i<weights.size();++i){
    if(weights[i]>maxweight){
     maxweight=weights[i];
     icenter=i;
    }
   }

// no more clusters to find
   if(maxweight<0.){break;}

// create the new cluster
   Ints newcluster=neighbors[icenter];
   clusters.push_back(newcluster);

// remove from pool
   for(unsigned i=0;i<newcluster.size();++i){
    unsigned k=0;
    while(k<neighbors.size()){
// eliminate the entire neighbor list
     if(neighbors[k][0]==newcluster[i]){
      neighbors.erase(neighbors.begin()+k);
      weights.erase(weights.begin()+k);
     }else{
// and the element in all the other neighbor lists
      std::vector<int>::iterator it=
       find (neighbors[k].begin(), neighbors[k].end(), newcluster[i]);
      if(it!=neighbors[k].end()){
       neighbors[k].erase(it);
       weights[k]-=d->get_weight(newcluster[i]);
      }
      k++;
     }
    }
   }
  }

  IMP_NEW(statistics::internal::TrivialPartitionalClustering,ret,(clusters));
  statistics::validate_partitional_clustering(ret, d->get_number_of_items());
  return ret.release();
}


IMPMEMBRANE_END_NAMESPACE
