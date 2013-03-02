/*   Test the GeometricHash class by comparing its results to those obtained
 *   by simple distance calculation.
 *
 *   Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/algebra/constants.h>
#include <IMP/multifit/GeometricHash.h>

typedef IMP::multifit::GeometricHash<int, 2> Hash2;
typedef std::vector<Hash2::Point> Points;
typedef std::vector<int> Indices;

namespace {
// square Euclidean distance
double sq_dist(const Hash2::Point &p1, const Hash2::Point &p2)
{
  double d0 = p1[0] - p2[0];
  double d1 = p1[1] - p2[1];
  return d0*d0 + d1*d1;
}

// find the indices of points within the distance of radius from the
// center by simple distance calculation
Indices dist_nearest(const Points &points, const Hash2::Point &center,
                     double radius)
{
  radius *= radius;
  Indices inds;
  for (unsigned i = 0; i < points.size(); ++i) {
    if (sq_dist(points[i], center) <= radius) {
      inds.push_back(i);
    }
  }
  return inds;
}


// find the indices of points within the distance of radius from the
// center using the hash
Indices hash_nearest(const Points &points, const Hash2::Point &center,
                     double radius)
{
  Hash2 H;
  for (unsigned i = 0; i < points.size(); ++i) {
    H.add(points[i], i);
  }
  Hash2::HashResult r = H.neighbors(H.EUCLIDEAN, center, radius);
  Indices ret;
  for (Hash2::HashResult::const_iterator it = r.begin(); it != r.end(); ++it) {
    ret.push_back((*it)->second);
  }
  return ret;
}

// generate some points
Points gen_points(int N)
{
  Points points;
  for (int r = 0; r < N; ++r) {
    for (int i = 0; i < N; ++i) {
      double angle = 2.0*i*IMP::algebra::PI/static_cast<float>(N);
      points.push_back(Hash2::Point(0.1*r*std::cos(angle),
                                    0.1*r*std::sin(angle)));
    }
  }
  return points;
}
}
int main()
{
  Points points = gen_points(200);
  Hash2::Point cnt(1.,1.);
  double rd = 0.45;
  Indices dist = dist_nearest(points, cnt, rd);
  Indices hash = hash_nearest(points, cnt, rd);

  std::sort(dist.begin(), dist.end());
  std::sort(hash.begin(), hash.end());

  if (dist.size() != hash.size()) {
    IMP_THROW("lists are different sizes", ValueException);
  }

  if (!std::equal(dist.begin(), dist.end(), hash.begin())) {
    IMP_THROW("lists have differing elements", ValueException);
  }

  return 0;
}
