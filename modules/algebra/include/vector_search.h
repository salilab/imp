/**
 *  \file vector_search.h   \brief Functions to generate vectors.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_VECTOR_SEARCH_H
#define IMPALGEBRA_VECTOR_SEARCH_H

#include "VectorD.h"


#ifdef IMP_USE_CGAL
#include <CGAL/basic.h>
#include <CGAL/Search_traits.h>
#include <CGAL/Orthogonal_k_neighbor_search.h>
#include <CGAL/K_neighbor_search.h>
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
  Ints linear_nearest_neighbor(const VectorD<D> &q, int skip,
                               unsigned int k) const {
    Ints ret;
    std::vector<double> retds;
    for (unsigned int i=0; i< data_.size(); ++i) {
      if (SKIP && i==static_cast<unsigned int>(skip)) continue;
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
    return ret;
  }

#ifdef IMP_USE_CGAL
  struct VectorWithIndex: public VectorD<D> {
    int index;
    VectorWithIndex(unsigned int i, const VectorD<D>& p): VectorD<D>(p),
                                                          index(i){}
    VectorWithIndex(): index(-1){}
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
    return linear_nearest_neighbor<false>(q, -1, 1)[0];
#endif
  }
  /** Search using the ith point in the input set. */
  unsigned int get_nearest_neighbor(unsigned int i) const {
#ifdef IMP_USE_CGAL
    K_neighbor_search search(*tree_, data_[i], 2, eps_);
    IMP_INTERNAL_CHECK(std::distance(search.begin(), search.end()) >=2,
                       "Wrong number of points returned "
                       << std::distance(search.begin(), search.end()));
    if (search.begin()->first.index != i) return search.begin()->first.index;
    else return (++search.begin())->first.index;
#else
    return linear_nearest_neighbor<true>(data_[i], i, 1)[0];
#endif
  }
  /** Search using the ith point in the input set. Return the k
      nearest neighbors.*/
  Ints get_nearest_neighbors(unsigned int i, unsigned int k) const {
    Ints ret(std::min(k, static_cast<unsigned int>(data_.size())));
#ifdef IMP_USE_CGAL
    K_neighbor_search search(*tree_, data_[i], k+1, eps_);
    typename K_neighbor_search::iterator it =search.begin();
    IMP_INTERNAL_CHECK(std::distance(search.begin(), search.end())
                       == static_cast<int>(ret.size()+1),
                       "Got the wrong number of points out from CGAL neighbor"
                       << " search. Expected " << ret.size()+1
                       << " got "
                       << std::distance(search.begin(), search.end()));
    ++it;
    for (unsigned int j=0; j< k; ++j) {
      ret[j]=it->first.index;
      ++it;
    }
#else
    ret= linear_nearest_neighbor<true>(data_[i], i, k);
#endif
    return ret;
  }
};

/** @} */

IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_VECTOR_SEARCH_H */
