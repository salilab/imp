/**
 *  \file IMP/statistics/embeddings.h
 *  \brief Cluster sets of points.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSTATISTICS_EMBEDDINGS_H
#define IMPSTATISTICS_EMBEDDINGS_H

#include <IMP/statistics/statistics_config.h>
#include "Embedding.h"
#include "embedding_macros.h"
#include <IMP/ConfigurationSet.h>
#include <IMP/SingletonContainer.h>
#include <IMP/algebra/VectorD.h>

IMPSTATISTICS_BEGIN_NAMESPACE

//! Embed a configuration using the XYZ coordinates of a set of particles
/** The point for each configuration of the model is a concatenation of
    the Cartesian coordinates of the particles contained in the passed
    SingletonContainer.

    See ConfigurationSet for more information about the input.

    \pythonexample{basic_optimization}
*/
class IMPSTATISTICSEXPORT ConfigurationSetXYZEmbedding: public Embedding {
  mutable Pointer<ConfigurationSet> cs_;
  IMP::OwnerPointer<SingletonContainer> sc_;
  bool align_;
public:
  /** If align is true, all the configurations are rigidly aligned with
      the first before generating their coordinates.
  */
  ConfigurationSetXYZEmbedding(ConfigurationSet *cs,
                               SingletonContainerAdaptor pi,
                               bool align=false);
  IMP_EMBEDDING(ConfigurationSetXYZEmbedding);
};


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
  base::Vector<FloatRange> ranges_;
public:
  ParticleEmbedding(const ParticlesTemp &ps,
                    const FloatKeys& ks
#if defined(IMP_DOXYGEN)
                    =core::XYZ::get_xyz_keys()
#else
                    = FloatKeys(IMP::kernel::internal::xyzr_keys,
                                IMP::kernel::internal::xyzr_keys+3)
#endif
,
                    bool rescale=false);
  IMP_EMBEDDING(ParticleEmbedding);
};


//! Simply return the coordinates of a VectorD
class IMPSTATISTICSEXPORT VectorDEmbedding: public Embedding {
  base::Vector<algebra::VectorKD > vectors_;
public:
  template <class C>
    VectorDEmbedding(const C &vs):
    Embedding("VectorDs"){
    vectors_.resize(vs.size());
    for (unsigned int i=0; i< vs.size(); ++i) {
      vectors_[i]= algebra::VectorKD(vs[i].coordinates_begin(),
                                     vs[i].coordinates_end());
    }
  }
#ifdef SWIG
  VectorDEmbedding(const algebra::VectorKDs &vs);
  VectorDEmbedding(const algebra::Vector2Ds &vs);
  VectorDEmbedding(const algebra::Vector3Ds &vs);
  VectorDEmbedding(const algebra::Vector4Ds &vs);
  VectorDEmbedding(const algebra::Vector5Ds &vs);
  VectorDEmbedding(const algebra::Vector6Ds &vs);
#endif
  IMP_EMBEDDING(VectorDEmbedding);
};


IMPSTATISTICS_END_NAMESPACE

#endif /* IMPSTATISTICS_EMBEDDINGS_H */
