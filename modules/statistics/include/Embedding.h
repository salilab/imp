/**
 *  \file IMP/statistics/Embedding.h
 *  \brief Cluster sets of points.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSTATISTICS_EMBEDDING_H
#define IMPSTATISTICS_EMBEDDING_H

#include <IMP/statistics/statistics_config.h>
#include <IMP/algebra/VectorD.h>
#include <IMP/Object.h>
#include <IMP/object_macros.h>

IMPSTATISTICS_BEGIN_NAMESPACE

//! Store data to be clustered for embedding based algorithms.
/** Point-based clustering needs a way of embedding the data being clustered
    in space. Classes which implement Embedding provide a
    mapping between each item being clustered (named by an integer index)
    and a point in space, as a fixed-length array of floating point numbers.
 */
class IMPSTATISTICSEXPORT Embedding : public IMP::Object {
 protected:
  Embedding(std::string name);
  //! By default return 1.0
  virtual double do_get_weight(unsigned int i) const {
    IMP_UNUSED(i);
    return 1.0;
  }

 public:
  virtual algebra::VectorKD get_point(unsigned int i) const = 0;
  virtual unsigned int get_number_of_items() const = 0;
  //! Return a weight for the point.
  double get_weight(unsigned int i) const { return do_get_weight(i); }
  virtual algebra::VectorKDs get_points() const {
    algebra::VectorKDs ret(get_number_of_items());
    for (unsigned int i = 0; i < ret.size(); ++i) {
      ret[i] = get_point(i);
    }
    return ret;
  }
};

IMP_OBJECTS(Embedding, Embeddings);

IMPSTATISTICS_END_NAMESPACE

#endif /* IMPSTATISTICS_EMBEDDING_H */
