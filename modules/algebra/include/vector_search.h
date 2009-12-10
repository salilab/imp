/**
 *  \file vector_search.h   \brief Functions to generate vectors.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_VECTOR_SEARCH_H
#define IMPALGEBRA_VECTOR_SEARCH_H

#include "VectorD.h"


#ifdef IMP_USE_CGAL
#include <CGAL/basic.h>
#include <CGAL/Search_traits.h>
#include <CGAL/Orthogonal_k_neighbor_search.h>
#include <boost/static_assert.hpp>
#endif

IMPALGEBRA_BEGIN_NAMESPACE

/** @name Vector Search

    These functions classes create various search structures
    over sets of vectors. Most of them benifit a great deal
    from having CGAL available.
    @{
 */

/** Build a structure for finding nearest neighbors. This is quite slow
    without CGAL.
    \ingroup{CGAL}
*/
template <unsigned int D>
class NearestNeighborD {

  template <bool SKIP>
  int linear_nearest_neighbor(const VectorD<D> &q, int skip) const {
    double md= std::numeric_limits<double>::max();
    int imax=-1;
    for (unsigned int i=0; i< data_.size(); ++i) {
      if (SKIP && i==skip) continue;
      double cd=(data_[i]-q).get_squared_magnitude();
      if (cd < md) {
        md= cd;
        imax=i;
      }
    }
    return imax;
  }

#ifdef IMP_USE_CGAL
  struct VectorWithIndex: public VectorD<D> {
    int index;
    VectorWithIndex(unsigned int i, VectorD<D> p): VectorD<D>(p), index(i){}
    VectorWithIndex(): index(-1){}
  };
  struct Construct_coord_iterator {
    const double* operator()(const VectorD<D>& p) const
    { return static_cast<const double*>(p.get_data()); }
    const double* operator()(const VectorD<D>& p, int)  const
    { return static_cast<const double*>(p.get_data()+D); }
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
      BOOST_STATIC_ASSERT(D==3);
      double distance(0.0), h = p[0];
      if (h < b.min_coord(0)) distance += (b.min_coord(0)-h)*(b.min_coord(0)-h);
      if (h > b.max_coord(0)) distance += (h-b.max_coord(0))*(h-b.max_coord(0));
      h=p[1];
      if (h < b.min_coord(1)) distance += (b.min_coord(1)-h)*(b.min_coord(1)-h);
      if (h > b.max_coord(1)) distance += (h-b.max_coord(1))*(h-b.min_coord(1));
      h=p[2];
      if (h < b.min_coord(2)) distance += (b.min_coord(2)-h)*(b.min_coord(2)-h);
      if (h > b.max_coord(2)) distance += (h-b.max_coord(2))*(h-b.max_coord(2));
      return distance;
    }

    template <class TreeTraits>
    double max_distance_to_rectangle(const VectorD<D>& p,
                         const CGAL::Kd_tree_rectangle<TreeTraits>& b) const {
      double h = p[0];
      double d0 = (h >= (b.min_coord(0)+b.max_coord(0))/2.0) ?
        (h-b.min_coord(0))*(h-b.min_coord(0))
        : (b.max_coord(0)-h)*(b.max_coord(0)-h);
      h=p[1];
      double d1 = (h >= (b.min_coord(1)+b.max_coord(1))/2.0) ?
        (h-b.min_coord(1))*(h-b.min_coord(1))
        : (b.max_coord(1)-h)*(b.max_coord(1)-h);
      h=p[2];
      double d2 = (h >= (b.min_coord(2)+b.max_coord(2))/2.0) ?
        (h-b.min_coord(2))*(h-b.min_coord(2))
        : (b.max_coord(2)-h)*(b.max_coord(2)-h);
      return d0 + d1 + d2;
    }
    double new_distance(double& dist, double old_off, double new_off,
                        int /* cutting_dimension */)  const {
      return dist + new_off*new_off - old_off*old_off;
    }
    double transformed_distance(double d) const { return d*d; }
    double inverse_of_transformed_distance(double d) { return std::sqrt(d); }
  }; // end of struct Distance

  typedef typename CGAL::Search_traits<double, VectorWithIndex,
                              const double*, Construct_coord_iterator> Traits;
  typedef typename CGAL::Orthogonal_k_neighbor_search<Traits,
                                                  Distance> K_neighbor_search;
  typedef typename K_neighbor_search::Tree Tree;

  struct RCTree: public Tree, public RefCounted {
    template <class It>
    RCTree(It b, It e): Tree(b,e){}
    virtual ~RCTree(){}
  };
  mutable Pointer<RCTree> tree_;
#endif
  std::vector<VectorD<D> > data_;
  double eps_;
public:
  NearestNeighborD(const std::vector<VectorD<D> > &vs,
                   double epsilon=0):
    data_(vs),
    eps_(epsilon)
 {
#ifdef IMP_USE_CGAL
    std::vector<VectorWithIndex> vsi(vs.size());
    for (unsigned int i=0; i< vs.size(); ++i) {
      vsi[i]= VectorWithIndex(i, vs[i]);
    }
    tree_= new RCTree(vsi.begin(), vsi.end());
#endif
  }

  unsigned int get_nearest_neighbor(const VectorD<D> &q) const {
#ifdef IMP_USE_CGAL
    K_neighbor_search search(*tree_, VectorWithIndex(-1, q), 1, eps_);
    return search.begin()->first.index;
#else
    return linear_nearest_neighbor<false>(q, -1);
#endif
  }
  unsigned int get_nearest_neighbor(unsigned int index) const {
#ifdef IMP_USE_CGAL
    K_neighbor_search search(*tree_, data_[index], 2, eps_);
    return (++search.begin())->first.index;
#else
    return linear_nearest_neighbor<true>(data_[index], index);
#endif
    }
};

/** @} */

IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_VECTOR_SEARCH_H */
