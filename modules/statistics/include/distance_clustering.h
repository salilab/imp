/**
 *  \file distance_clustering.h
 *  \brief Cluster sets of points.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMPSTATISTICS_DISTANCE_CLUSTERING_H
#define IMPSTATISTICS_DISTANCE_CLUSTERING_H

#include "statistics_config.h"
#include "statistics_macros.h"
#include "PartitionalClustering.h"
#include <IMP/algebra/VectorD.h>
#include <IMP/macros.h>
#include <IMP/VectorOfRefCounted.h>
#include <IMP/ConfigurationSet.h>
#include <IMP/SingletonContainer.h>
#include <IMP/internal/OwnerPointer.h>

IMPSTATISTICS_BEGIN_NAMESPACE

//! Compute a distance between two elements to be clustered
/** Distance-based clustering needs a way of computing the
    distances between the things being clustered.
 */
class IMPSTATISTICSEXPORT Distance: public Object {
public:
  Distance(std::string name);
  virtual double get_distance(unsigned int i,
                              unsigned int j) const =0;
  virtual unsigned int get_number_of_items() const=0;
  IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(Distance);
};

IMP_OBJECTS(Distance, Distances);

class IMPSTATISTICSEXPORT EuclideanDistance: public Distance {
  algebra::VectorKDs vectors_;
public:
  template <int D>
  EuclideanDistance(const std::vector<algebra::VectorD<D> > &vs):
    Distance("VectorDs"){
    vectors_.resize(vs.size());
    for (unsigned int i=0; i< vs.size(); ++i) {
      vectors_[i]= algebra::VectorKD(vs[i].coordinates_begin(),
                                     vs[i].coordinates_end());
    }
  }
#ifdef SWIG
  EuclideanDistance(const algebra::VectorKDs &vs);
  EuclideanDistance(const algebra::Vector2Ds &vs);
  EuclideanDistance(const algebra::Vector3Ds &vs);
  EuclideanDistance(const algebra::Vector4Ds &vs);
  EuclideanDistance(const algebra::Vector5Ds &vs);
  EuclideanDistance(const algebra::Vector6Ds &vs);
#endif
  IMP_DISTANCE(EuclideanDistance);
};


/** Compute the RMSD between two sets of particles in two configurations.
 */
class IMPSTATISTICSEXPORT ConfigurationSetRMSDistance: public Distance {
  IMP::internal::OwnerPointer<ConfigurationSet> cs_;
  IMP::internal::OwnerPointer<SingletonContainer> sc_;
  bool align_;
 public:
  ConfigurationSetRMSDistance(ConfigurationSet *cs,
                              SingletonContainer *sc,
                              bool align=false);
  IMP_DISTANCE(ConfigurationSetRMSDistance);
};



/** Cluster by repeatedly removing edges which have lots
    of shortest paths passing through them. The process is
    terminated when there are a set number of
    connected components. Other termination criteria
    can be added if someone proposes them.
 */
IMPSTATISTICSEXPORT
PartitionalClustering *get_centrality_clustering(Distance *d,
                                                 double far,
                                                 int k);

IMPSTATISTICS_END_NAMESPACE

#endif /* IMPSTATISTICS_DISTANCE_CLUSTERING_H */
