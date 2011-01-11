/**
 *  \file internal/ann.h
 *  \brief manipulation of text, and Interconversion between text and numbers
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
*/

#ifndef IMPALGEBRA_INTERNAL_ANN_H
#define IMPALGEBRA_INTERNAL_ANN_H

#include "../algebra_config.h"
#ifdef IMP_ALGEBRA_USE_ANN
#include <ANN/ANN.h>
#include <IMP/base_types.h>
#include <IMP/log.h>
#include <boost/scoped_array.hpp>

IMPALGEBRA_BEGIN_INTERNAL_NAMESPACE

template <unsigned int D>
struct ANNData {
  boost::scoped_array<ANNcoord*> points_;
  mutable ANNkd_tree tree_;
  double fix_distance(double dist, double eps) const {
    return (1+eps)*dist;
  }
  ANNData(){}
  template <class It>
  ANNcoord** extract_points(It b, It e) const {
    unsigned int i=0;
    ANNcoord** ret = new ANNcoord*[std::distance(b,e)];
    for (It c=b; c != e; ++c) {
      ANNcoord *pt= new ANNcoord[D];
      VectorD<D> v= get_vector_d_geometry(*c);
      std::copy(v.coordinates_begin(), v.coordinates_end(), pt);
      ret[i++]=pt;
    }
    return ret;
  }
  template <class It>
  ANNData(It b, It e): points_(extract_points(b,e)),
                       tree_(points_.get(), std::distance(b,e), D){
  }
  template <class G>
  void fill_nearest_neighbors(const G &g, unsigned int k,
                              double eps, Ints&ret) const {
    VectorD<D> v= get_vector_d_geometry(g);
    ANNcoord pt[D];
    std::copy(v.coordinates_begin(), v.coordinates_end(), pt);
    boost::scoped_array<ANNdist> dists(new ANNdist[k]);
    IMP_INTERNAL_CHECK(ret.size() >=k, "Not large enough array");
    tree_.annkSearch(pt, std::min(k, get_number_of_points()),
                     &ret[0], dists.get(), eps);
  }
  template <class G>
  void fill_nearest_neighbors(const G &g, double distance,
                              double eps, Ints&ret) const {
    static const unsigned int guess=20;
    static double guess_dists[guess];
    ret.resize(guess);
    VectorD<D> v= get_vector_d_geometry(g);
    ANNcoord pt[D];
    std::copy(v.coordinates_begin(), v.coordinates_end(), pt);
    unsigned int k=tree_.annkFRSearch(pt, square(fix_distance(distance, eps)),
                                      guess, &ret[0], guess_dists, eps);
    ret.resize(k);
    if (k >= guess) {
      IMP_LOG(VERBOSE, "falling back on second nn search " << k << std::endl);
      boost::scoped_array<ANNdist> dists(new ANNdist[k]);
      tree_.annkFRSearch(pt, square(fix_distance(distance, eps)), k,
                         &ret[0], dists.get(), eps);
    }
  }
  VectorD<D> get_point(unsigned int i) const {
    return VectorD<D>(tree_.thePoints()[i], tree_.thePoints()[i]+D);
  }
  unsigned int get_number_of_points() const {
    return tree_.nPoints();
  }
  ~ANNData() {
    for (int i=0; i< tree_.nPoints(); ++i) {
      delete[] tree_.thePoints()[i];
    }
  }
};

IMPALGEBRA_END_INTERNAL_NAMESPACE

#endif // IMP_USE_ANN

#endif  /* IMPALGEBRA_INTERNAL_ANN_H */
