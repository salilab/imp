/**
 *  \file internal/cgal_knn.h
 *  \brief manipulation of text, and Interconversion between text and numbers
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
*/

#include <IMP/cgal/internal/knn.h>
#include <CGAL/basic.h>
#include <CGAL/Search_traits.h>
#include <CGAL/Orthogonal_k_neighbor_search.h>
#include <CGAL/K_neighbor_search.h>
#include <CGAL/Fuzzy_sphere.h>
#include <boost/static_assert.hpp>
#include <IMP/RefCounted.h>
#include <IMP/Pointer.h>


IMPCGAL_BEGIN_INTERNAL_NAMESPACE

RCTree::~RCTree(){}

template <unsigned int D>
struct Construct_coord_iterator {
  const double* operator()(const algebra::VectorD<D>& p) const
  { return p.get_data(); }
  const double* operator()(const algebra::VectorD<D>& p, int)  const
  { return p.get_data()+D; }
};
template <unsigned int D>
struct Distance {
  typedef algebra::VectorD<D> Query_item;

  double transformed_distance(const algebra::VectorD<D>& p1,
                              const algebra::VectorD<D>& p2) const {
    return (p1-p2).get_squared_magnitude();
  }

  template <class TreeTraits>
  double min_distance_to_rectangle(const algebra::VectorD<D>& p,
                  const CGAL::Kd_tree_rectangle<TreeTraits>& b) const {
    double distance(0.0);
    for (unsigned int i=0; i< D; ++i) {
      double h = p[i];
      if (h < b.min_coord(i)) distance += square(b.min_coord(i)-h);
      if (h > b.max_coord(i)) distance += square(h-b.max_coord(i));
    }
    return distance;
  }

  template <class TreeTraits>
  double max_distance_to_rectangle(const algebra::VectorD<D>& p,
                    const CGAL::Kd_tree_rectangle<TreeTraits>& b) const {
    double d=0.0;
    for (unsigned int i=0; i< D; ++i) {
      double h = p[i];
      double di = (h >= (b.min_coord(i)+b.max_coord(i))/2.0) ?
        square(h-b.min_coord(i)) : square(b.max_coord(i)-h);
      d+=di;
    }
    return d;
  }
  double new_distance(double dist, double old_off, double new_off,
                      int /* cutting_dimension */)  const {
    return dist + new_off*new_off - old_off*old_off;
  }
  double transformed_distance(double d) const { return d*d; }
  double inverse_of_transformed_distance(double d) { return std::sqrt(d); }
}; // end of struct Distance

template <unsigned int D>
struct RealRCTree: public RCTree {
  typedef typename CGAL::Search_traits<double, VectorWithIndex<D>,
                                       const double*,
                                       Construct_coord_iterator<D> > Traits;
  typedef typename CGAL::Fuzzy_sphere<Traits> Sphere;
  typedef typename CGAL::K_neighbor_search<Traits, Distance<D> >
  K_neighbor_search;
  typedef typename K_neighbor_search::Tree Tree;
  Tree tree;
  template <class It>
  RealRCTree(It b, It e): tree(b,e){}
};

#define IMP_CGAL_KNN_D_DEF(D)                                           \
  KNNData##D::KNNData##D(const std::vector<VectorWithIndex<D> > &v):    \
  vsi_(v) {                                                             \
    tree_= new RealRCTree<D>(v.begin(), v.end());                       \
  }                                                                     \
  void KNNData##D::fill_nearest_neighbors_v(const algebra::VectorD<D> &g, \
                                          unsigned int k,               \
                                          double eps,                   \
                                          Ints &ret) const {            \
    VectorWithIndex<D> d(-1, g);                                        \
    RealRCTree<D>::                                                     \
      K_neighbor_search search(dynamic_cast<RealRCTree<D>*>(tree_.get()) \
                             ->tree,                                    \
                             d, k, eps);                                \
    IMP_INTERNAL_CHECK(std::distance(search.begin(), search.end())      \
                       == static_cast<int>(k),                          \
                       "Got the wrong number of points out from CGAL neighbor" \
                       << " search. Expected " << k                     \
                       << " got "                                       \
                       << std::distance(search.begin(), search.end())); \
    Ints::iterator rit = ret.begin();                                   \
    for ( RealRCTree<D>::K_neighbor_search::iterator it = search.begin(); \
         it != search.end(); ++it) {                                    \
      *rit= it->first;                                                  \
      ++rit;                                                            \
    }                                                                   \
  }                                                                     \
  void KNNData##D::fill_nearest_neighbors_v(const algebra::VectorD<D> &g, \
                                            double dist,                \
                                            double eps,                 \
                                            Ints &ret) const {          \
    VectorWithIndex<D> d(-1, g);                                        \
    dynamic_cast<RealRCTree<D>*>(tree_.get())                           \
      ->tree.search(std::back_inserter(ret),                            \
                    RealRCTree<D>::Sphere(d, dist, eps));               \
  }                                                                     \


IMP_CGAL_KNN_D_DEF(2)
IMP_CGAL_KNN_D_DEF(3)
IMP_CGAL_KNN_D_DEF(4)

IMPCGAL_END_INTERNAL_NAMESPACE
