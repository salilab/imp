/**
 *  \file IMP/multifit/GeometricHash.h   \brief Geometric Hashing class.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_GEOMETRIC_HASH_H
#define IMPMULTIFIT_GEOMETRIC_HASH_H

#include <vector>
#include <map>
#include <ostream>
#include <cmath>
#include <boost/array.hpp>
#include <IMP/multifit/multifit_config.h>
#include "IMP/algebra/VectorD.h"

IMPMULTIFIT_BEGIN_NAMESPACE

//! Geometric Hash table.
/** GeometricHash<T, D> stores values of type T with D-dimensional
    points whose coordinates are of type double inside D-dimensional
    buckets (cubes) */
template <typename T, size_t D>
class GeometricHash {
 public:
  enum Distance {
    INF,
    EUCLIDEAN
  };
  typedef algebra::VectorD<D> Point;
  typedef boost::array<int, D> Bucket;
  typedef std::pair<Point, T> ValueType;
  typedef std::vector<ValueType> PointList;
  typedef std::map<const Bucket, PointList> GeomMap;
  typedef typename GeomMap::iterator iterator;
  typedef typename GeomMap::const_iterator const_iterator;
  typedef std::vector<const ValueType *> HashResult;
  typedef IMP::base::Vector<T> HashResultT;
  typedef std::vector<Bucket> BucketList;

  /* Default constructor - all cubes/buckets have the same edge length */
  GeometricHash(double radius = 1) {
    for (size_t i = 0; i < D; ++i) radii_[i] = radius;
  }

  /* This constructor allows to specify the length of cube edges per axis */
  template <typename X>
  GeometricHash(const X &radii) {
    for (size_t i = 0; i < D; ++i) radii_[i] = radii[i];
  }

  // For Swig
  GeometricHash(const algebra::VectorD<D> &radii) {
    for (size_t i = 0; i < D; ++i) radii_[i] = radii[i];
  }

  /* Add a point with the associated value into the hash map */
  void add(const Point &pt, const T &data) {
    Bucket b = to_bucket(pt);
    gmap_[b].push_back(std::make_pair(pt, data));
  }

  /* Add a point with the associated value into the hash map */
  template <typename X>
  void add(const X &coordinates, const T &data) {
    Point pt;
    for (size_t i = 0; i < D; ++i) pt[i] = coordinates[i];
    Bucket b = to_bucket(pt);
    gmap_[b].push_back(std::make_pair(pt, data));
  }

  /* Find all points that are within radius distance from center.
   * The vector returned have elements that point to (point, value)
   * pairs that are in the neighborhood. Distance specify which norm
   * to use (Euclidean or l-infinity). The result is invalidated
   * whenever add function is called. */
  HashResult neighbors(Distance dt, const Point &center, double radius) const {
    if (dt == EUCLIDEAN)
      return points_in_sphere(inside_sphere(center, radius));
    else
      return points_in_sphere(inside_sphere_inf(center, radius));
  }

  // For Swig
  HashResultT neighbors_data(Distance dt, const Point &center,
                             double radius) const {
    HashResult r = neighbors(dt, center, radius);
    HashResultT tr;
    tr.reserve(r.size());
    for (size_t i = 0; i < r.size(); ++i) tr.push_back(r[i]->second);
    return tr;
  }

  template <typename X>
  HashResult neighbors(Distance dt, const X &center_coordinates,
                       double radius) const {
    Point center;
    for (size_t i = 0; i < D; ++i) center[i] = center_coordinates[i];
    if (dt == EUCLIDEAN)
      return points_in_sphere(inside_sphere(center, radius));
    else
      return points_in_sphere(inside_sphere_inf(center, radius));
  }

  void neighbors(Distance dt, const Point &center, double radius,
                 HashResultT &values) {
    values.clear();
    HashResult nb = neighbors(dt, center, radius);
    values.reserve(nb.size());
    for (size_t i = 0; i < nb.size(); ++i) values.push_back(nb[i]->second);
  }

  /* Find all points inside a cube centered in center with edges
   * radii*2 long */
  template <typename X>
  HashResult neighbors(const Point &center, const X &radii) const {
    return points_in_sphere(inside_cube(center, radii));
  }

  /* Find all points inside a cube centered in center with edges
   * 2 times bigger than the grid */
  HashResult neighbors(const Point &center) {
    return neighbors(center, radii_);
  }

  template <typename X>
  HashResult neighbors(const X &center_coordinates) {
    Point center;
    for (size_t i = 0; i < D; ++i) center[i] = center_coordinates[i];
    return neighbors(center, radii_);
  }

  /* Return the hash map */
  GeomMap const &Map() const { return gmap_; }

  iterator begin() { return gmap_.begin(); }

  iterator end() { return gmap_.end(); }

  const_iterator begin() const { return gmap_.begin(); }

  const_iterator end() const { return gmap_.end(); }

  /* Return vector of buckets */
  BucketList get_buckets() const {
    BucketList result;
    for (typename GeomMap::const_iterator p = gmap_.begin(); p != gmap_.end();
         ++p)
      result.push_back(p->first);
    return result;
  }

  /* Remove all data from the hash map */
  void clear() { gmap_.clear(); }

  PointList const *bucket(const Point &pt) const {
    typename GeomMap::const_iterator p = gmap_.find(to_bucket(pt));
    if (p != gmap_.end()) return &p->second;
    return 0;
  }

  double resolution(int axis = 0) const { return radii_[axis]; }

 private:
  struct inside_sphere {
    inside_sphere(const Point &center, double radius)
        : center_(center), radius_(radius), sq_radius_(radius * radius) {}

    bool operator()(const Point &pt) const {
      return get_squared_distance(center_, pt) <= sq_radius_;
    }

    double radius(int) const { return radius_; }

    const Point &center_;
    double radius_;
    double sq_radius_;
  };

  struct inside_sphere_inf {
    inside_sphere_inf(const Point &center, double radius)
        : center_(center), radius_(radius) {}

    bool operator()(const Point &pt) const {
      return inf_distance(center_, pt) <= radius_;
    }

    double radius(int) const { return radius_; }

    const Point &center_;
    double radius_;
  };

  struct inside_cube {
    template <typename X>
    inside_cube(const Point &center, const X &radii)
        : center_(center) {
      for (size_t i = 0; i < D; ++i) radii_[i] = radii[i];
    }

    bool operator()(const Point &pt) const {
      for (size_t i = 0; i < D; ++i)
        if (std::fabs(pt[i] - center_[i]) > radii_[i]) return false;
      return true;
    }

    double radius(size_t i) const { return radii_[i]; }

    const Point &center_;
    double radii_[D];
  };

  template <typename Dist>
  HashResult points_in_sphere(Dist const &dist) const {
    Point C;
    for (size_t i = 0; i < D; ++i) C[i] = dist.center_[i] - dist.radius(i);
    Bucket l = to_bucket(C);
    for (size_t i = 0; i < D; ++i) C[i] = dist.center_[i] + dist.radius(i);
    Bucket u = to_bucket(C);
    HashResult result;
    Bucket tmp;
    points_in_sphere_rec(0, l, u, dist, tmp, result);
    return result;
  }

  template <typename Dist>
  bool cube_inside_sphere(const Dist &ins, const Bucket &l) const {
    Point p = from_bucket(l);
    return cube_inside_sphere_rec(ins, p, 0);
  }

  template <typename Dist>
  bool cube_inside_sphere_rec(const Dist &ins, Point &p, size_t idx) const {
    if (idx >= D) return ins(p);
    if (!cube_inside_sphere_rec(ins, p, idx + 1)) return false;
    double old = p[idx];
    p[idx] += radii_[idx];
    bool r = cube_inside_sphere_rec(ins, p, idx + 1);
    p[idx] = old;
    return r;
  }

  template <typename Dist>
  void points_in_sphere_rec(size_t idx, const Bucket &l, const Bucket &u,
                            const Dist &ins, Bucket &tmp,
                            HashResult &result) const {
    if (idx >= D) {
      typename GeomMap::const_iterator p = gmap_.find(tmp);
      if (p != gmap_.end()) {
        const PointList &v = p->second;
        if (v.size() > (1 << D) && cube_inside_sphere(ins, tmp)) {
          for (typename PointList::const_iterator q = v.begin(); q != v.end();
               ++q)
            result.push_back(&(*q));
        } else {
          for (typename PointList::const_iterator q = v.begin(); q != v.end();
               ++q)
            if (ins(q->first)) result.push_back(&(*q));
        }
      }
      return;
    }
    for (int i = l[idx]; i <= u[idx]; ++i) {
      tmp[idx] = i;
      points_in_sphere_rec(idx + 1, l, u, ins, tmp, result);
    }
  }

  /* Create bucket containing the given point p */
  Bucket to_bucket(const Point &p) const {
    Bucket r;
    for (size_t i = 0; i < D; ++i) {
      if (p[i] < 0)
        r[i] = int(p[i] / radii_[i]) - 1;
      else
        r[i] = int(p[i] / radii_[i]);
    }
    return r;
  }

  /* Find the vertex of bucket b with the smallest coordinates */
  Point from_bucket(const Bucket &b) const {
    Point p(b.begin(), b.end());
    for (size_t i = 0; i < D; ++i) p[i] *= radii_[i];
    return p;
  }

  /* l-infinity distance */
  static double inf_distance(const Point &p1, const Point &p2) {
    double r = std::fabs(p1[0] - p2[0]);
    for (size_t i = 1; i < D; ++i) {
      double d = std::fabs(p1[i] - p2[i]);
      if (r < d) r = d;
    }
    return r;
  }

  GeomMap gmap_;
  double radii_[D];
};

IMPMULTIFIT_END_NAMESPACE
#endif /* IMPMULTIFIT_GEOMETRIC_HASH_H */
