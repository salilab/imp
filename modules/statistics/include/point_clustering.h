/**
 *  \file point_clustering.h
 *  \brief Cluster sets of points.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */
#ifndef IMPSTATISTICS_POINT_CLUSTERING_H
#define IMPSTATISTICS_POINT_CLUSTERING_H

#include "config.h"
#include "macros.h"
#include "PartitionalClustering.h"
#include <IMP/macros.h>
#include <IMP/Object.h>
#include <IMP/ConfigurationSet.h>
#include <IMP/SingletonContainer.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/em/DensityMap.h>
#include <IMP/core/XYZ.h>

IMPSTATISTICS_BEGIN_NAMESPACE

//! Map clustering data to spatial positions
/** Point-based clustering needs a way of embedding the data being clustered
    in space. Classes which implement Embedding provide a
    mapping between each item being clustered (named by an integer index)
    and a point in space, as a fixed-lenth array of floating point numbers.
 */
class IMPSTATISTICSEXPORT Embedding: public Object {
public:
  Embedding(std::string name): Object(name){}
  virtual Floats get_point(unsigned int i) const =0;
  virtual unsigned int get_number_of_points() const=0;
  IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(Embedding);
};


//! Embed a configuration using the XYZ coordinates of a set of particles
/** The point for each configuration of the model is a concatenation of
    the cartesian coordinates of the particles contained in the passed
    SingletonContainer.

    See ConfigurationSet for more information about the input.
*/
class IMPSTATISTICSEXPORT ConfigurationSetXYZEmbedding {
  mutable Pointer<ConfigurationSet> cs_;
  Pointer<SingletonContainer> sc_;
public:
  ConfigurationSetXYZEmbedding(ConfigurationSet *cs,
                               SingletonContainer *sc);
  IMP_EMBEDDING(ConfigurationSetXYZEmbedding, get_module_version_info());
};



//! Simply return the coordinates of a VectorD
template <unsigned int D>
class VectorDEmbedding: public Embedding {
  std::vector<algebra::VectorD<D> > vectors_;
public:
  VectorDEmbedding(const std::vector<algebra::VectorD<D> > &vs):
    Embedding("VectorDs"), vectors_(vs){}
  IMP_EMBEDDING(VectorDEmbedding, get_module_version_info());
};

#ifndef SWIG
template <unsigned int D>
Floats VectorDEmbedding<D>::get_point(unsigned int i) const {
  return Floats(vectors_[i].coordinates_begin(),
                vectors_[i].coordinates_end());
}

template <unsigned int D>
unsigned int VectorDEmbedding<D>::get_number_of_points() const {
  return vectors_.size();
}

template <unsigned int D>
void VectorDEmbedding<D>::show(std::ostream &out) const {
  out << "VectorDEmbedding" << std::endl;
}
#endif

/** Embed particles using the values of some of their attributes.
    By default, the cartesian coordinates are used, but another
    set of attributes can be chosen. When using attributes that
    are not equivalent (for example, angular degrees of freedom),
    it is probably useful to rescale the attributes according
    to their ranges (see IMP::Model::get_range()). This is
    done by passing rescale=true to the constructor.
*/
class IMPSTATISTICSEXPORT ParticleEmbedding: public Embedding {
  Particles ps_;
  FloatKeys ks_;
  bool rescale_;
  std::vector<FloatRange> ranges_;
public:
  ParticleEmbedding(const ParticlesTemp &ps,
                    const FloatKeys& ks=core::XYZ::get_xyz_keys(),
                    bool rescale=false);
  IMP_EMBEDDING(ParticleEmbedding, get_module_version_info());
};



/** Generate a set of points from the voxels in a em::DensityMap
    which are above a certain threshold.
 */
class IMPSTATISTICSEXPORT HighDensityEmbedding: public Embedding {
  algebra::Vector3Ds points_;
public:
  HighDensityEmbedding(em::DensityMap *dm,
                       double threshold);
  IMP_EMBEDDING(HighDensityEmbedding, get_module_version_info());
};



/** In addition to the information in the Clustering base class,
    KMeansClustering stores a cluster center for each cluster.
    The cluster center is a point in the space defined by the
    embedding.

    The representative for each cluster is the member whose
    location in the embedding is closest to the cluster center.
*/
class IMPSTATISTICSEXPORT KMeansClustering: public PartitionalClustering {
  std::vector<Ints> clusters_;
  Ints reps_;
  std::vector<Floats> centers_;
public:
  KMeansClustering(const std::vector<Ints> &clusters,
                   const std::vector<Floats> &centers,
                   const Ints &reps): PartitionalClustering("k-means"),
                                      clusters_(clusters),
                                      reps_(reps),
                                      centers_(centers){}
  const Floats& get_cluster_center(unsigned int i) const {
    return centers_[i];
  }
  IMP_CLUSTERING(KMeansClustering, get_module_version_info());
};


/** Return a k-means clustering of all points contained in the
    embedding (ie [0... embedding->get_number_of_embeddings())).
    These points are then clustered into k clusters. More iterations
    takes longer but produces a better clustering.
*/
IMPSTATISTICSEXPORT KMeansClustering*
get_lloyds_kmeans(Embedding *embedding,
                  unsigned int k, unsigned int iterations);

IMPSTATISTICS_END_NAMESPACE

#endif /* IMPSTATISTICS_POINT_CLUSTERING_H */
