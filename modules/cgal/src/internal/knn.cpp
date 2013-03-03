/**
 *  \file internal/cgal_knn.h
 *  \brief manipulation of text, and Interconversion between text and numbers
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
*/
#include <IMP/cgal/internal/knn.h>
#include <CGAL/basic.h>
#include <CGAL/Search_traits.h>
#include <CGAL/Orthogonal_k_neighbor_search.h>
#include <CGAL/K_neighbor_search.h>
#include <CGAL/Fuzzy_sphere.h>
#include <boost/static_assert.hpp>
#include <IMP/base/utility.h>
#include <limits>


IMPCGAL_BEGIN_INTERNAL_NAMESPACE

RCTree::~RCTree(){}

struct Construct_coord_iterator {
  const double* operator()(const algebra::VectorKD& p) const
  { return p.get_data(); }
  const double* operator()(const algebra::VectorKD& p, int)  const
  { return p.get_data()+p.get_dimension(); }
  typedef const double* result_type;
};
struct Distance {
  typedef algebra::VectorKD Query_item;

  double transformed_distance(const algebra::VectorKD& p1,
                              const algebra::VectorKD& p2) const {
    return (p1-p2).get_squared_magnitude();
  }

  template <class TreeTraits>
  double min_distance_to_rectangle(const algebra::VectorKD& p,
                  const CGAL::Kd_tree_rectangle<TreeTraits>& b) const {
    double distance(0.0);
    for (unsigned int i=0; i< p.get_dimension(); ++i) {
      double h = p[i];
      if (h < b.min_coord(i)) distance += base::square(b.min_coord(i)-h);
      if (h > b.max_coord(i)) distance += base::square(h-b.max_coord(i));
    }
    return distance;
  }

  template <class TreeTraits>
  double max_distance_to_rectangle(const algebra::VectorKD& p,
                    const CGAL::Kd_tree_rectangle<TreeTraits>& b) const {
    double d=0.0;
    for (unsigned int i=0; i< p.get_dimension(); ++i) {
      double h = p[i];
      double di = (h >= (b.min_coord(i)+b.max_coord(i))/2.0) ?
        base::square(h-b.min_coord(i)) : base::square(b.max_coord(i)-h);
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

struct RealRCTree: public RCTree {
  typedef  CGAL::Search_traits<double, VectorWithIndex,
                                       const double*,
                                       Construct_coord_iterator > Traits;
  typedef  CGAL::Fuzzy_sphere<Traits> Sphere;
  typedef  CGAL::K_neighbor_search<Traits, Distance >
  K_neighbor_search;
  typedef  K_neighbor_search::Tree Tree;
  Tree tree;
  template <class It>
  RealRCTree(It b, It e): tree(b,e){}
};

void KNNData::initialize(const base::Vector<VectorWithIndex > &v) {
  vsi_=v;
  tree_= new RealRCTree(v.begin(), v.end());
}
void KNNData::fill_nearest_neighbors_v(const algebra::VectorKD &g,
                                       unsigned int k,
                                       double eps,
                                       Ints &ret) const {
  VectorWithIndex d(std::numeric_limits<int>::max(), g);
  RealRCTree::
    K_neighbor_search search(dynamic_cast<RealRCTree*>(tree_.get())
                             ->tree,
                             d, k, eps);
  IMP_IF_CHECK(base::USAGE_AND_INTERNAL) {
    int nump=std::distance(dynamic_cast<RealRCTree*>(tree_.get())
                           ->tree.begin(),
                           dynamic_cast<RealRCTree*>(tree_.get())
                           ->tree.end());
    int realk=std::min<int>(nump, k);
    IMP_CHECK_VARIABLE(realk);
    IMP_INTERNAL_CHECK(std::distance(search.begin(), search.end())
                       == static_cast<int>(realk),
                       "Got the wrong number of points out from CGAL neighbor"
                       << " search. Expected " << realk
                       << " got "
                       << std::distance(search.begin(), search.end()));
  }
  Ints::iterator rit = ret.begin();
  for ( RealRCTree::K_neighbor_search::iterator it = search.begin();
        it != search.end(); ++it) {
    *rit= it->first;
    ++rit;
  }
}
void KNNData::fill_nearest_neighbors_v(const algebra::VectorKD &g,
                                          double dist,
                                          double eps,
                                          Ints &ret) const {
  VectorWithIndex d(std::numeric_limits<int>::max(), g);
  dynamic_cast<RealRCTree*>(tree_.get())
    ->tree.search(std::back_inserter(ret),
                  RealRCTree::Sphere(d, dist, eps));
}



IMPCGAL_END_INTERNAL_NAMESPACE
