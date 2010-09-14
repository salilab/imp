/**
 *  \file cgal/internal/knn.h
 *  \brief manipulation of text, and Interconversion between text and numbers
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#ifndef IMPCGAL_INTERNAL_KNN_H
#define IMPCGAL_INTERNAL_KNN_H

#include "../cgal_config.h"
#include <IMP/base_types.h>
#include <IMP/Pointer.h>
#include <IMP/algebra/VectorD.h>
#include <boost/static_assert.hpp>
#include <IMP/RefCounted.h>


IMPCGAL_BEGIN_INTERNAL_NAMESPACE
template <unsigned int D>
struct VectorWithIndex: public algebra::VectorD<D> {
  int index;
  VectorWithIndex(unsigned int i, const algebra::VectorD<D>& p):
    algebra::VectorD<D>(p),
    index(i){}
  VectorWithIndex(): index(-1){}
  operator unsigned int() const {return index;}
  unsigned int dimension() const {
    return D;
  }
};

template <unsigned int D, class It>
std::vector<VectorWithIndex<D> > create_vectors_with_index(It b, It e) {
  std::vector<VectorWithIndex<D> > v(std::distance(b,e));
  It c=b;
  for (unsigned int i=0; i< v.size(); ++i) {
    v[i]= VectorWithIndex<D>(i, get_vector_d_geometry(*c));
    ++c;
  }
  return v;
}

struct IMPCGALEXPORT RCTree: public RefCounted {
  virtual ~RCTree();
};


template <unsigned int D>
struct KNNData{};

#define IMP_CGAL_KNN_D(D)                                               \
  struct IMPCGALEXPORT KNNData##D {                                     \
    mutable Pointer<RCTree> tree_;                                      \
    std::vector<VectorWithIndex<D> > vsi_;                              \
    KNNData##D(const std::vector<VectorWithIndex<D> > &v);              \
    void fill_nearest_neighbors_v(const algebra::VectorD<D> &g,         \
                                  unsigned int k,                       \
                                  double eps, Ints &ret) const;         \
    void fill_nearest_neighbors_v(const algebra::VectorD<D> &g,         \
                                  double dist,                          \
                                  double eps, Ints &ret) const;         \
    template <class G>                                                  \
    void fill_nearest_neighbors(const G &g, unsigned int k,             \
                                double eps, Ints &ret) const {          \
      fill_nearest_neighbors_v(get_vector_d_geometry(g),                \
                               k, eps, ret);                            \
    }                                                                   \
    template <class G>                                                  \
    void fill_nearest_neighbors(const G &g, double distance,            \
                                double eps, Ints &ret) const {          \
      fill_nearest_neighbors_v(get_vector_d_geometry(g),                \
                               distance, eps, ret);                     \
    }                                                                   \
    const algebra::VectorD<D> &get_point(unsigned int i) const {        \
      return vsi_[i];                                                   \
    }                                                                   \
    unsigned int get_number_of_points() const {                         \
      return vsi_.size();                                               \
    }                                                                   \
  };                                                                    \
  template <>                                                           \
  struct KNNData<D>: public KNNData##D {                                \
    template <class It>                                                 \
    KNNData(It b, It e): KNNData##D(create_vectors_with_index<D>(b,e)) { \
    }                                                                   \
  }                                                                     \

IMP_CGAL_KNN_D(2);
IMP_CGAL_KNN_D(3);
IMP_CGAL_KNN_D(4);

IMPCGAL_END_INTERNAL_NAMESPACE

#endif  /* IMPCGAL_INTERNAL_KNN_H */
