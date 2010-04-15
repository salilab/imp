/**
 *  \file internal/linear_knn.h
 *  \brief manipulation of text, and Interconversion between text and numbers
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#ifndef IMPALGEBRA_INTERNAL_LINEAR_KNN_H
#define IMPALGEBRA_INTERNAL_LINEAR_KNN_H

#include "../algebra_config.h"


IMPALGEBRA_BEGIN_INTERNAL_NAMESPACE

template <unsigned int D>
struct LinearKNNData {
  void linear_nearest_neighbor(IMP_RESTRICT const VectorD<D> &q,
                               unsigned int k, Ints &ret) const {
    std::vector<double> retds;
    for (unsigned int i=0; i< data_.size(); ++i) {
      double cd=(data_[i]-q).get_squared_magnitude();
      if (ret.size() < k || cd < retds.back()) {
        std::vector<double>::iterator it= std::lower_bound(retds.begin(),
                                                           retds.end(), cd);
        ret.insert(ret.begin()+(it-retds.begin()), i);
        retds.insert(it, cd);
        if (ret.size() > k) {
          ret.pop_back();
          retds.pop_back();
        }
      }
    }
  }

  std::vector<VectorD<D> > data_;
  template <class It>
  LinearKNNData(It b, It e) {
    data_.reserve(std::distance(b,e));
    for (It c= b; c != e; ++c) {
      data_.push_back(get_geometry(*c));
    }
  }
  template <class G>
  void fill_nearest_neighbors(const G &g, unsigned int k,
                              double , Ints &ret) const {
    linear_nearest_neighbor(get_geometry(g), k, ret);
  }
  template <class G>
  void fill_nearest_neighbors(const G &g, double distance,
                              double , Ints &ret) const {
    double d2= square(distance);
    VectorD<D> v= get_geometry(g);
    for (unsigned int i=0; i< data_.size(); ++i) {
      if (get_squared_distance(v, data_[i]) < d2) {
        ret.push_back(i);
      }
    }
  }
  const VectorD<D>& get_point(unsigned int i) const {
    return data_[i];
  }
  unsigned int get_number_of_points() const {
    return data_.size();
  }
};

IMPALGEBRA_END_INTERNAL_NAMESPACE


#endif  /* IMPALGEBRA_INTERNAL_LINEAR_KNN_H */
