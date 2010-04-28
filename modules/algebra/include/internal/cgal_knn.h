/**
 *  \file internal/cgal_knn.h
 *  \brief manipulation of text, and Interconversion between text and numbers
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#ifndef IMPALGEBRA_INTERNAL_CGAL_KNN_H
#define IMPALGEBRA_INTERNAL_CGAL_KNN_H

#include "../algebra_config.h"
#ifdef IMP_USE_CGAL
#include <CGAL/basic.h>
#include <CGAL/Search_traits.h>
#include <CGAL/Orthogonal_k_neighbor_search.h>
#include <CGAL/K_neighbor_search.h>
#include <boost/static_assert.hpp>
#include <IMP/RefCounted.h>
#include <IMP/Pointer.h>


IMPALGEBRA_BEGIN_INTERNAL_NAMESPACE

template <unsigned int D>
struct CGALKNNData {
  struct VectorWithIndex: public VectorD<D> {
    int index;
    VectorWithIndex(unsigned int i, const VectorD<D>& p): VectorD<D>(p),
                                                          index(i){}
    VectorWithIndex(): index(-1){}
    operator unsigned int() const {return index;}
  };
  struct Construct_coord_iterator {
    const double* operator()(const VectorD<D>& p) const
    { return p.get_data(); }
    const double* operator()(const VectorD<D>& p, int)  const
    { return p.get_data()+D; }
  };
  struct Distance {
    typedef VectorD<D> Query_item;

    double transformed_distance(const VectorD<D>& p1,
                                const VectorD<D>& p2) const {
      return (p1-p2).get_squared_magnitude();
    }

    template <class TreeTraits>
    double min_distance_to_rectangle(const VectorD<D>& p,
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
    double max_distance_to_rectangle(const VectorD<D>& p,
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

  typedef typename CGAL::Search_traits<double, VectorWithIndex,
                              const double*, Construct_coord_iterator> Traits;
  typedef typename CGAL::K_neighbor_search<Traits,
                                                  Distance> K_neighbor_search;
  typedef typename K_neighbor_search::Tree Tree;

  struct RCTree: public Tree, public RefCounted {
    template <class It>
    RCTree(It b, It e): Tree(b,e){}
    virtual ~RCTree(){}
  };
  mutable Pointer<RCTree> tree_;
  std::vector<VectorWithIndex> vsi_;


  template <class It>
  CGALKNNData(It b, It e): vsi_(std::distance(b,e)) {
    It c=b;
    for (unsigned int i=0; i< vsi_.size(); ++i) {
      vsi_[i]= VectorWithIndex(i, get_vector_d_geometry(*c));
      ++c;
    }
    tree_= new RCTree(vsi_.begin(), vsi_.end());
  }
  template <class G>
  void fill_nearest_neighbors(const G &g, unsigned int k,
                              double eps, Ints &ret) const {
    VectorWithIndex d(-1, get_vector_d_geometry(g));
    K_neighbor_search search(*tree_, d, k, eps);
    IMP_INTERNAL_CHECK(std::distance(search.begin(), search.end())
                       == static_cast<int>(k),
                       "Got the wrong number of points out from CGAL neighbor"
                       << " search. Expected " << k
                       << " got "
                       << std::distance(search.begin(), search.end()));
    typename Ints::iterator rit = ret.begin();
    for (typename K_neighbor_search::iterator it = search.begin();
         it != search.end(); ++it) {
      *rit= it->first;
      ++rit;
    }
  }
  template <class G>
  void fill_nearest_neighbors(const G &g, double distance,
                              double eps, Ints &ret) const {
    IMP_NOT_IMPLEMENTED;
  }
  const VectorD<D> &get_point(unsigned int i) const {
    return vsi_[i];
  }
  unsigned int get_number_of_points() const {
    return vsi_.size();
  }
};

IMPALGEBRA_END_INTERNAL_NAMESPACE

#endif // IMP_USE_CGAL

#endif  /* IMPALGEBRA_INTERNAL_CGAL_KNN_H */
