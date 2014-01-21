/**
 *  \file internal/linear_knn.h
 *  \brief manipulation of text, and Interconversion between text and numbers
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
*/

#ifndef IMPALGEBRA_INTERNAL_LINEAR_KNN_H
#define IMPALGEBRA_INTERNAL_LINEAR_KNN_H

#include <IMP/algebra/algebra_config.h>
#include "MinimalSet.h"

IMPALGEBRA_BEGIN_INTERNAL_NAMESPACE

template <int D>
struct LinearKNNData {
  void linear_nearest_neighbor(const VectorD<D> &q, unsigned int k,
                               Ints &ret) const {
    MinimalSet<double, int> retds(k);
    for (unsigned int i = 0; i < data_.size(); ++i) {
      double cd = (data_[i] - q).get_squared_magnitude();
      if (retds.can_insert(cd)) {
        retds.insert(cd, i);
      }
    }
    for (unsigned int i = 0; i < retds.size(); ++i) {
      ret[i] = retds[i].second;
    }
  }

  base::Vector<VectorD<D> > data_;
  template <class It>
  LinearKNNData(It b, It e) {
    data_.reserve(std::distance(b, e));
    for (It c = b; c != e; ++c) {
      data_.push_back(get_vector_geometry(*c));
    }
  }
  template <class G>
  void fill_nearest_neighbors(const G &g, unsigned int k, double,
                              Ints &ret) const {
    linear_nearest_neighbor(get_vector_geometry(g), k, ret);
  }
  template <class G>
  void fill_nearest_neighbors(const G &g, double distance, double,
                              Ints &ret) const {
    double d2 = base::square(distance);
    VectorD<D> v = get_vector_geometry(g);
    for (unsigned int i = 0; i < data_.size(); ++i) {
      if (get_squared_distance(v, data_[i]) < d2) {
        ret.push_back(i);
      }
    }
  }
  const VectorD<D> &get_point(unsigned int i) const { return data_[i]; }
  unsigned int get_number_of_points() const { return data_.size(); }
};

IMPALGEBRA_END_INTERNAL_NAMESPACE

#endif /* IMPALGEBRA_INTERNAL_LINEAR_KNN_H */
