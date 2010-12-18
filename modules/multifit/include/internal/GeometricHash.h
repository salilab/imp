/**
 *  \file GeometricHash.h   \brief Geometric Hashing class.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMPMULTIFIT_GEOMETRIC_HASH_H
#define IMPMULTIFIT_GEOMETRIC_HASH_H

#include <vector>
#include <map>
#include <ostream>
#include <cmath>
#include "../multifit_config.h"

IMPMULTIFIT_BEGIN_INTERNAL_NAMESPACE

/* Point<F, D> represents a D-dimensional point whose coordinates have
    * type F.
    * Point<F, D> can be initialized from any container that supports []
    * operator. For example:
    * std::vector<float> v(3);
    * ...
    * Point<float, 3> P(v);
    * The coordinates of Point can be accessed using []. Point<int, D> is
    * used to store bucket coordinates. */
template < typename F, int D >
class Point
{
public:
  typedef F data_type;

  Point() {}

  template < typename T >
  Point(const T &init)
  {
    for ( int i=0; i<D; ++i )
      coord_[i] = init[i];
  }

  // For Swig...
  Point(const std::vector<F> &init)
  {
    for ( int i=0; i<D; ++i )
      coord_[i] = init[i];
  }

  F &operator[](int k)
  {
    return coord_[k];
  }

  const F &operator[](int k) const
  {
    return coord_[k];
  }

private:
  F coord_[D];
};


/* Ordering relation for points (essentially "dictionary" ordering). */
template < typename F, int D >
bool operator<(const Point<F, D> &p1, const Point<F, D> &p2)
{
  for ( int i=0; i<D; ++i )
    if ( p1[i] < p2[i] )
      return true;
    else if ( p1[i] > p2[i] )
      return false;
  return false;
}


/* Definition of equality */
template < typename F, int D >
bool operator==(const Point<F, D> &p1, const Point<F, D> &p2)
{
  for ( int i=0; i<D; ++i )
    if ( p1[i] != p2[i] )
      return false;
  return true;
}


/* Squared Euclidean distance */
template < typename F, int D >
F sq_distance(const Point<F, D> &p1, const Point<F, D> &p2)
{
  F r = 0;
  for ( int i=0; i<D; ++i )
  {
    F d = p1[i] - p2[i];
    r += d*d;
  }
  return r;
}


/* l-infinity distance */
template < typename F, int D >
F inf_distance(const Point<F, D> &p1, const Point<F, D> &p2)
{
  F r = std::fabs(p1[0] - p2[0]);
  for ( int i=1; i<D; ++i )
  {
    F d = std::fabs(p1[i] - p2[i]);
    if ( r < d )
      r = d;
  }
  return r;
}


/* Output Point into a stream */
template < typename F, int D >
std::ostream &operator<<(std::ostream &os, const Point<F, D> &p)
{
  os << '(';
  for ( int i=0; i<D; ++i )
  {
    if ( i > 0 )
      os << ", ";
    os << p[i];
  }
  os << ')';
  return os;
}


/* This is the definition of Geometric Hash table.
    * GeometricHash<T, F, D> stores values of type T with D-dimensional
    * points whose coordinates are of type F inside D-dimensional
    * buckets (cubes) */
template < typename T, typename F, int D >
class GeometricHash
{
public:
  enum Distance { INF, EUCLIDEAN };
  typedef Point<F, D> GeoPoint;
  typedef Point<int32_t, D> Bucket;
  typedef std::pair< GeoPoint, T > ValueType;
  typedef const ValueType *Placeholder;
  typedef std::vector< ValueType > PointList;
  typedef std::map<Bucket, PointList> GeomMap;
  typedef std::vector<Placeholder> HashResult;
  typedef std::vector<T> HashResultT;


  /* Default constructor - all cubes/buckets have the same edge length */
  GeometricHash(F radius = 1)
  {
    for ( int i=0; i<D; ++i )
      radii_[i] = radius;
  }

  /* This constructor allows to specify the length of cube edges per axis */
  template < typename X >
  GeometricHash(const X &radii)
  {
    for ( int i=0; i<D; ++i )
      radii_[i] = radii[i];
  }

  // For Swig
  GeometricHash(const std::vector<F> &radii)
  {
    for ( int i=0; i<D; ++i )
      radii_[i] = radii[i];
  }


  /* Add a point with the associated value into the hash map */
  void add(const GeoPoint &pt, const T &data)
  {
    Bucket b = to_bucket(pt);
    gmap_[b].push_back(std::make_pair(pt, data));
  }

  // For Swig...
  void add(const std::vector<F> &pt, const T &data)
  {
    add(GeoPoint(pt), data);
  }

  /* Find all points that are within radius distance from center.
   * The vector returned have elements that point to (point, value)
   * pairs that are in the neighborhood. Distance specify which norm
   * to use (Euclidean or l-infinity). The result is invalidated
   * whenever add function is called. */
  HashResult neighbors(Distance dt, const GeoPoint &center,
      F radius) const
  {
    if ( dt == EUCLIDEAN )
      return points_in_sphere(inside_sphere(center, radius));
    else
      return points_in_sphere(inside_sphere_inf(center, radius));
  }

  // For Swig
  HashResultT neighbors(Distance dt, const std::vector<F> &center,
      F radius) const
  {
    HashResult r = neighbors(dt, GeoPoint(center), radius);
    HashResultT tr;
    tr.reserve(r.size());
    for ( size_t i=0; i<r.size(); ++i )
      tr.push_back(r[i]->second);
    return tr;
  }

  /* Find all points inside a cube centered in center with edges
   * radii*2 long */
  template < typename X >
  HashResult neighbors(const GeoPoint &center,
      const X &radii) const
  {
    return points_in_sphere(inside_cube(center, radii));
  }

  /* Find all points inside a cube centered in center with edges
   * 2 times bigger than the grid */
  HashResult neighbors(const GeoPoint &center)
  {
    return neighbors(center, radii_);
  }

  /* Return the hash map */
  GeomMap const &Map() const
  {
    return gmap_;
  }

  /* Remove all data from the hash map */
  void clear()
  {
    gmap_.clear();
  }

  PointList const *bucket(const GeoPoint &pt) const
  {
    typename GeomMap::const_iterator p = gmap_.find(to_bucket(pt));
    if ( p != gmap_.end() )
      return &p->second;
    return 0;
  }

  F resolution(int axis = 0) const
  {
    return radii_[axis];
  }


private:

  struct inside_sphere
  {
    inside_sphere(const GeoPoint &center, F radius)
      : center_(center)
      , radius_(radius)
      , sq_radius_(radius*radius)
    {}

    bool operator()(const GeoPoint &pt) const
    {
      return sq_distance(center_, pt) <= sq_radius_;
    }

    F radius(int) const
    {
      return radius_;
    }

    const GeoPoint &center_;
    F radius_;
    F sq_radius_;
  };

  struct inside_sphere_inf
  {
    inside_sphere_inf(const GeoPoint &center, F radius)
      : center_(center)
      , radius_(radius)
    {}

    bool operator()(const GeoPoint &pt) const
    {
      return inf_distance(center_, pt) <= radius_;
    }

    F radius(int) const
    {
      return radius_;
    }

    const GeoPoint &center_;
    F radius_;
  };

  struct inside_cube
  {
    template < typename X >
    inside_cube(const GeoPoint &center, const X &radii)
      : center_(center)
    {
      for ( int i=0; i<D; ++i )
        radii_[i] = radii[i];
    }

    bool operator()(const GeoPoint &pt) const
    {
      for ( int i=0; i<D; ++i )
        if ( std::fabs(pt[i] - center_[i]) > radii_[i] )
          return false;
      return true;
    }

    F radius(int i) const
    {
      return radii_[i];
    }

    const GeoPoint &center_;
    F radii_[D];
  };

  template < typename Dist >
  HashResult points_in_sphere(Dist const &dist) const
  {
    F C[D];
    for ( int i=0; i<D; ++i )
      C[i] = dist.center_[i] - dist.radius(i);
    Bucket l = to_bucket(GeoPoint(C));
    for ( int i=0; i<D; ++i )
      C[i] = dist.center_[i] + dist.radius(i);
    Bucket u = to_bucket(GeoPoint(C));
    HashResult result;
    Bucket tmp;
    points_in_sphere_rec(0, l, u, dist, tmp, result);
    return result;
  }

  template < typename Dist >
  bool cube_inside_sphere(const Dist &ins, const Bucket &l) const
  {
    GeoPoint p = from_bucket(l);
    return cube_inside_sphere_rec(ins, p, 0);
  }

  template < typename Dist >
  bool cube_inside_sphere_rec(const Dist &ins,
      GeoPoint &p, int idx) const
  {
    if ( idx >= D )
      return ins(p);
    if (! cube_inside_sphere_rec(ins, p, idx + 1) )
      return false;
    F old = p[idx];
    p[idx] += radii_[idx];
    bool r = cube_inside_sphere_rec(ins, p, idx + 1);
    p[idx] = old;
    return r;
  }

  template < typename Dist >
  void points_in_sphere_rec(int idx, const Bucket &l, const Bucket &u,
      const Dist &ins, Bucket &tmp,
      HashResult &result) const
  {
    if ( idx >= D )
    {
      typename GeomMap::const_iterator p = gmap_.find(tmp);
      if ( p != gmap_.end() )
      {
        const PointList &v = p->second;
        if ( v.size() > (1<<D) && cube_inside_sphere(ins, tmp) )
        {
          for ( typename PointList::const_iterator q = v.begin();
              q != v.end(); ++q )
          result.push_back(&(*q));
        }
        else
        {
          for ( typename PointList::const_iterator q = v.begin();
              q != v.end(); ++q )
            if ( ins(q->first) )
              result.push_back(&(*q));
        }
      }
      return;
    }
    for ( int i=l[idx]; i<=u[idx]; ++i )
    {
      tmp[idx] = i;
      points_in_sphere_rec(idx + 1, l, u, ins, tmp, result);
    }
  }

  /* Create bucket containing the given point p */
  Bucket to_bucket(const GeoPoint &p) const
  {
    Bucket r;
    for ( int i=0; i<D; ++i )
    {
      if ( p[i] < 0 )
        r[i] = int(p[i]/radii_[i]) - 1;
      else
        r[i] = int(p[i]/radii_[i]);
    }
    return r;
  }

  /* Find the vertex of bucket b with the smallest coordinates */
  GeoPoint from_bucket(const Bucket &b) const
  {
    GeoPoint p(b);
    for ( int i=0; i<D; ++i )
      p[i] *= radii_[i];
    return p;
  }

  GeomMap gmap_;
  F radii_[D];
};

IMPMULTIFIT_END_INTERNAL_NAMESPACE
#endif  /* IMPMULTIFIT_GEOMETRIC_HASH_H */
