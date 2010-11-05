/**
 *  \file point_clustering.h
 *  \brief Cluster sets of points.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMPSTATISTICS_POINT_CLUSTERING_H
#define IMPSTATISTICS_POINT_CLUSTERING_H

#include "statistics_config.h"
#include "statistics_macros.h"
#include "PartitionalClustering.h"
#include <IMP/macros.h>
#include <IMP/Object.h>
#include <IMP/ConfigurationSet.h>
#include <IMP/SingletonContainer.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/em/DensityMap.h>
#include <IMP/algebra/VectorD.h>
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
    the Cartesian coordinates of the particles contained in the passed
    SingletonContainer.

    See ConfigurationSet for more information about the input.

    \htmlinclude basic_optimization.py.html
*/
class IMPSTATISTICSEXPORT ConfigurationSetXYZEmbedding: public Embedding {
  mutable Pointer<ConfigurationSet> cs_;
  IMP::internal::OwnerPointer<SingletonContainer> sc_;
  bool align_;
public:
  ConfigurationSetXYZEmbedding(ConfigurationSet *cs,
                               SingletonContainer *sc,
                               bool align=false);
  IMP_EMBEDDING(ConfigurationSetXYZEmbedding);
};



//! Simply return the coordinates of a VectorD
template <unsigned int D>
class VectorDEmbedding: public Embedding {
  std::vector<algebra::VectorD<D> > vectors_;
public:
  VectorDEmbedding(const std::vector<algebra::VectorD<D> > &vs):
    Embedding("VectorDs"), vectors_(vs){}
  IMP_EMBEDDING(VectorDEmbedding);
};

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
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
void VectorDEmbedding<D>::do_show(std::ostream &out) const {
}
#endif

/** Embed particles using the values of some of their attributes.
    By default, the Cartesian coordinates are used, but another
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
  IMP_EMBEDDING(ParticleEmbedding);
};


/** In addition to the information in the Clustering base class,
    PartitionalClusteringWithCenter stores a cluster center for
    each cluster.
    The cluster center is a point in the space defined by the
    embedding.

    The representative for each cluster is the member whose
    location in the embedding is closest to the cluster center.
*/
class IMPSTATISTICSEXPORT PartitionalClusteringWithCenter:
  public PartitionalClustering {
  std::vector<Ints> clusters_;
  Ints reps_;
  std::vector<Floats> centers_;
public:
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  template <unsigned int D>
    PartitionalClusteringWithCenter(const std::vector<Ints> &clusters,
                     const std::vector< algebra::VectorD<D> > &centers,
                     const Ints &reps):
    PartitionalClustering("k-means"),
    clusters_(clusters),
    reps_(reps),
    centers_(reps.size()){
    for (unsigned int i=0; i< centers_.size(); ++i) {
        centers_[i]= Floats(centers[i].coordinates_begin(),
                            centers[i].coordinates_end());
      }
  }
#endif
  PartitionalClusteringWithCenter(const std::vector<Ints> &clusters,
                   const std::vector<Floats> &centers,
                   const Ints &reps): PartitionalClustering("k-means"),
                                      clusters_(clusters),
                                      reps_(reps),
                                      centers_(centers){}
  const Floats& get_cluster_center(unsigned int i) const {
    return centers_[i];
  }
  IMP_CLUSTERING(PartitionalClusteringWithCenter);
};


/** Return a k-means clustering of all points contained in the
    embedding (ie [0... embedding->get_number_of_embeddings())).
    These points are then clustered into k clusters. More iterations
    takes longer but produces a better clustering.
*/
IMPSTATISTICSEXPORT PartitionalClusteringWithCenter*
get_lloyds_kmeans(Embedding *embedding,
                  unsigned int k, unsigned int iterations);

/** Two points, \f$p_i\f$, \f$p_j\f$ are in the same cluster if
    there is a sequence of points \f$\left(p^{ij}_{0}\dotsp^{ij}_k\right)\f$
    such that \f$\forall l ||p^{ij}_l-p^{ij}_{l+1}|| < d\f$.
 */
IMPSTATISTICSEXPORT PartitionalClusteringWithCenter*
get_connectivity_clustering(Embedding *embed,
                            double dist);

IMPSTATISTICS_END_NAMESPACE

#endif /* IMPSTATISTICS_POINT_CLUSTERING_H */
