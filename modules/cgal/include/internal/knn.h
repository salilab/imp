/**
 *  \file cgal/internal/knn.h
 *  \brief manipulation of text, and Interconversion between text and numbers
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCGAL_INTERNAL_KNN_H
#define IMPCGAL_INTERNAL_KNN_H

#include <IMP/cgal/cgal_config.h>
#include <IMP/base/types.h>
#include <IMP/base/Pointer.h>
#include <IMP/algebra/VectorD.h>
#include <boost/static_assert.hpp>
#include <IMP/base/Object.h>

IMPCGAL_BEGIN_INTERNAL_NAMESPACE
struct VectorWithIndex : public algebra::VectorKD {
  int index;
  VectorWithIndex(unsigned int i, const algebra::VectorKD &p)
      : algebra::VectorKD(p), index(i) {}
  VectorWithIndex() : index(-1) {}
  operator unsigned int() const { return index; }
  unsigned int dimension() const { return algebra::VectorKD::get_dimension(); }
};

template <class It>
base::Vector<VectorWithIndex> create_vectors_with_index(It b, It e) {
  base::Vector<VectorWithIndex> v(std::distance(b, e));
  It c = b;
  for (unsigned int i = 0; i < v.size(); ++i) {
    v[i] = VectorWithIndex(i, get_vector_geometry(*c));
    ++c;
  }
  return v;
}

struct IMPCGALEXPORT RCTree : public base::Object {
  RCTree() : Object("RCTree%1%") {}
  virtual ~RCTree();
};

struct IMPCGALEXPORT KNNData {
  mutable base::Pointer<RCTree> tree_;
  base::Vector<VectorWithIndex> vsi_;
  template <class It>
  KNNData(It b, It e) {
    initialize(create_vectors_with_index(b, e));
  }
  void initialize(const base::Vector<VectorWithIndex> &v);
  void fill_nearest_neighbors_v(const algebra::VectorKD &g, unsigned int k,
                                double eps, Ints &ret) const;
  void fill_nearest_neighbors_v(const algebra::VectorKD &g, double dist,
                                double eps, Ints &ret) const;
  template <class G>
  void fill_nearest_neighbors(const G &g, unsigned int k, double eps,
                              Ints &ret) const {
    fill_nearest_neighbors_v(get_vector_geometry(g), k, eps,
                             ret);
  }
  template <class G>
  void fill_nearest_neighbors(const G &g, double distance, double eps,
                              Ints &ret) const {
    fill_nearest_neighbors_v(get_vector_geometry(g), distance,
                             eps, ret);
  }
  const algebra::VectorKD &get_point(unsigned int i) const { return vsi_[i]; }
  unsigned int get_number_of_points() const { return vsi_.size(); }
};

IMPCGAL_END_INTERNAL_NAMESPACE

#endif /* IMPCGAL_INTERNAL_KNN_H */
