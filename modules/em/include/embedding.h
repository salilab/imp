/**
 *  \file embedding.h
 *  \brief Cluster sets of points.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMPEM_EMBEDDING_H
#define IMPEM_EMBEDDING_H

#include "em_config.h"
#include "DensityMap.h"
#include <IMP/statistics/point_clustering.h>

IMPEM_BEGIN_NAMESPACE


/** Generate a set of points from the voxels in a em::DensityMap
    which are above a certain threshold.
 */
class IMPEMEXPORT HighDensityEmbedding: public statistics::Embedding {
  std::vector<algebra::VectorD<3> > points_;
public:
  HighDensityEmbedding(DensityMap *dm,
                       double threshold);
  IMP_EMBEDDING(HighDensityEmbedding);
};

IMP_OBJECTS(HighDensityEmbedding, HighDensityEmbeddings);

IMPEM_END_NAMESPACE

#endif /* IMPEM_EMBEDDING_H */
