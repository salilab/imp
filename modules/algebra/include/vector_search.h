/**
 *  \file IMP/algebra/vector_search.h   \brief Functions to generate vectors.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_VECTOR_SEARCH_H
#define IMPALGEBRA_VECTOR_SEARCH_H

#include "VectorD.h"
#include <IMP/base/types.h>
#include <IMP/base/Object.h>
#include <IMP/base/log.h>
#include <IMP/base/SetCheckState.h>

#include "grid_storages.h"
#include "grid_ranges.h"
#include "GridD.h"
#ifdef IMP_ALGEBRA_USE_IMP_CGAL
#include <IMP/cgal/internal/knn.h>
#endif
#ifdef IMP_ALGEBRA_USE_ANN
#include "internal/ann.h"
#endif
#include "internal/linear_knn.h"
#include <fstream>

#ifdef IMP_ALGEBRA_USE_ANN
#define IMP_KNN_DATA internal::ANNData
#elif defined(IMP_ALGEBRA_USE_IMP_CGAL)
#define IMP_KNN_DATA IMP::cgal::internal::KNNData
#else
#define IMP_KNN_DATA internal::LinearKNNData<D>
#endif


IMPALGEBRA_BEGIN_NAMESPACE

/** @name Vector Search

    These functions classes create various search structures
    over sets of vectors.
    @{
*/

/** Build a structure for finding nearest neighbors. Different
    implementations are used depending on what optional dependencies
    are available.
    \uses{class NearestNeighborD, CGAL}
    \uses{class NearestNeighborD, ANN}
 */
template <int D>
class NearestNeighborD: public IMP::base::Object {
  IMP_KNN_DATA data_;
  double eps_;
#if IMP_HAS_CHECKS >= IMP_INTERNAL
  mutable std::ofstream query_log_;
#endif
  template <class It>
  void instantiate(It b, It e) {
    if (0) {
      // compile all of them
      Ints ret;
      internal::LinearKNNData<D> linear(b,e);
      linear.fill_nearest_neighbors(*b, 3U, eps_, ret);
    }
  }
public:
  template <class It>
  NearestNeighborD(It b, It e, double epsilon=0):
    Object("NearestNeighbor%1%"),
    data_(b,e), eps_(epsilon){
    instantiate(b,e);
  }
  NearestNeighborD(const base::Vector<VectorD<D> > &vs,
                   double epsilon=0):
    Object("NearestNeighbor%1%"),
    data_(vs.begin(), vs.end()),
    eps_(epsilon) {
    instantiate(vs.begin(), vs.end());
  }
  //! Log the points and queries to a file for performance studies
  void set_query_log(std::string fname) {
    IMP_INTERNAL_CHECK_VARIABLE(fname);
    IMP_OBJECT_LOG;
    set_was_used(true);
#if IMP_HAS_CHECKS >= IMP_INTERNAL
    query_log_.open(fname.c_str());
    for (unsigned int i=0; i< data_.get_number_of_points(); ++i) {
      query_log_ << spaces_io(data_.get_point(i)) << std::endl;
    }
    query_log_ << std::endl;
#endif
  }

  unsigned int get_nearest_neighbor(const VectorD<D> &q) const {
    IMP_OBJECT_LOG;
    set_was_used(true);
#if IMP_HAS_CHECKS >= IMP_INTERNAL
    if (query_log_) {
      query_log_ << spaces_io(q) << " " << 1 << std::endl;
    }
#endif
    Ints ret(1);
    data_.fill_nearest_neighbors(q, 1U, eps_, ret);
    return ret[0];
  }
  /** Search using the ith point in the input set. */
  unsigned int get_nearest_neighbor(unsigned int i) const {
    IMP_OBJECT_LOG;
    set_was_used(true);
#if IMP_HAS_CHECKS >= IMP_INTERNAL
    if (query_log_) {
      query_log_ << i << " " << 1 << std::endl;
    }
#endif
    Ints ret(2);
    data_.fill_nearest_neighbors(data_.get_point(i), 2U, eps_, ret);
    return ret[1];
  }
  /** Search using the ith point in the input set. Return the k
      nearest neighbors.*/
  Ints get_nearest_neighbors(unsigned int i, unsigned int k) const {
    IMP_OBJECT_LOG;
    set_was_used(true);
#if IMP_HAS_CHECKS >= IMP_INTERNAL
    if (query_log_) {
      query_log_ << i << " " << k << std::endl;
    }
#endif
    Ints ret(k+1);
    data_.fill_nearest_neighbors(data_.get_point(i), k+1, eps_, ret);
    return Ints(++ret.begin(), ret.end());
  }
  Ints get_nearest_neighbors(const algebra::VectorD<D> &v,
                             unsigned int k) const {
    IMP_OBJECT_LOG;
    set_was_used(true);
#if IMP_HAS_CHECKS >= IMP_INTERNAL
    if (query_log_) {
      query_log_ << v << " " << k << std::endl;
    }
#endif
    Ints ret(k+1);
    data_.fill_nearest_neighbors(v, k, eps_, ret);
    return Ints(++ret.begin(), ret.end());
  }
  /** Find all points within the provided distance. */
  Ints get_in_ball(unsigned int i, double distance) const {
    IMP_OBJECT_LOG;
    set_was_used(true);
    Ints ret;
    data_.fill_nearest_neighbors(data_.get_point(i), distance, eps_, ret);
    return Ints(++ret.begin(), ret.end());
  }
  /** Find all points within the provided distance. */
  Ints get_in_ball(const VectorD<D> &pt, double distance) const {
    IMP_OBJECT_LOG;
    set_was_used(true);
    Ints ret;
    data_.fill_nearest_neighbors(pt, distance, eps_, ret);
    return ret;
  }
  IMP_OBJECT_METHODS(NearestNeighborD);
};

/** @} */

#ifndef IMP_DOXYGEN
typedef NearestNeighborD<1> NearestNeighbor1D;
typedef NearestNeighborD<2> NearestNeighbor2D;
typedef NearestNeighborD<3> NearestNeighbor3D;
typedef NearestNeighborD<4> NearestNeighbor4D;
typedef NearestNeighborD<5> NearestNeighbor5D;
typedef NearestNeighborD<6> NearestNeighbor6D;
typedef NearestNeighborD<-1> NearestNeighborKD;
#endif



/** This class provides an incremental nearest neighbor search function.
    It's interface and behavior is somewhat different than that of
    NearestNeighborD, so be aware.

    Later this can support balls by copying points multiple times.
*/
class IMPALGEBRAEXPORT DynamicNearestNeighbor3D: public base::Object {
  typedef GridD<3, SparseGridStorageD<3, Ints,
      UnboundedGridRangeD<3> >, Ints,
      DefaultEmbeddingD<3> > Grid;
  Grid grid_;
  typedef Grid::Index Index;
  typedef Grid::ExtendedIndex EIndex;
  Vector3Ds coords_;
  base::Vector<Index> indexes_;
  void audit() const;
  void set_coordinates_internal(int id, Vector3D nc);
 public:
  DynamicNearestNeighbor3D(const Vector3Ds &vs,
                           double query_estimate=1);
  Ints get_in_ball(int id, double distance) const;
  void set_coordinates(int id, Vector3D nc);
  IMP_OBJECT_METHODS(DynamicNearestNeighbor3D);
};
IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_VECTOR_SEARCH_H */
