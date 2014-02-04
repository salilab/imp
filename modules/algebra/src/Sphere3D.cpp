/**
 *  \file  Sphere3D.cpp
 *  \brief simple implementation of spheres in 3D
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */
#include "IMP/algebra/Sphere3D.h"
#include <IMP/base/Index.h>
#include <IMP/base/exception.h>
#include <utility>
#ifdef IMP_ALGEBRA_USE_IMP_CGAL
#include <IMP/cgal/internal/bounding_sphere.h>
#include <IMP/cgal/internal/union_of_balls.h>
#endif

#include <IMP/algebra/vector_generators.h>
#include <IMP/algebra/vector_search.h>
#include <IMP/base/set_map_macros.h>

IMPALGEBRA_BEGIN_NAMESPACE

Sphere3D get_enclosing_sphere(const Sphere3Ds &ss) {
  IMP_USAGE_CHECK(!ss.empty(),
                  "Must pass some spheres to have a bounding sphere");
#ifdef IMP_ALGEBRA_USE_IMP_CGAL
  return cgal::internal::get_enclosing_sphere(ss);
#else
  BoundingBox3D bb = get_bounding_box(ss[0]);
  for (unsigned int i = 1; i < ss.size(); ++i) {
    bb += get_bounding_box(ss[i]);
  }
  Vector3D c = .5 * (bb.get_corner(0) + bb.get_corner(1));
  double r = 0;
  for (unsigned int i = 0; i < ss.size(); ++i) {
    double d = (c - ss[i].get_center()).get_magnitude();
    d += ss[i].get_radius();
    r = std::max(r, d);
  }
  return Sphere3D(c, r);
#endif
}

Sphere3D get_enclosing_sphere(const Vector3Ds &vs) {
  Sphere3Ds ss(vs.size());
  for (unsigned int i = 0; i < vs.size(); ++i) {
    ss[i] = Sphere3D(vs[i], 0);
  }
  return get_enclosing_sphere(ss);
}

#ifdef IMP_ALGEBRA_USE_IMP_CGAL
std::pair<double, double> get_surface_area_and_volume(
    const algebra::Sphere3Ds &ss) {
  return IMP::cgal::internal::get_surface_area_and_volume(ss);
}
#endif

namespace {

struct SphereTag {};
struct SPTag {};
typedef IMP::base::Index<SphereTag> SphereIndex;
typedef IMP::base::Index<SPTag> SPIndex;
typedef IMP::base::Vector<SphereIndex> SphereIndexes;
typedef IMP::base::Vector<SPIndex> SPIndexes;
typedef IMP_BASE_SMALL_UNORDERED_SET<SphereIndex> SphereIndexSet;
typedef IMP_BASE_SMALL_UNORDERED_SET<SPIndex> SPIndexSet;
}

Sphere3Ds get_simplified_from_volume(Sphere3Ds in,
                                     double maximum_allowed_error_angstroms) {
  IMP_FUNCTION_LOG;
  const double resolution = 1.0 / maximum_allowed_error_angstroms;
  const double probe = maximum_allowed_error_angstroms;

  IMP_FOREACH(Sphere3D & s, in) {
    s = Sphere3D(s.get_center(), s.get_radius() + probe);
  }

  Vector3Ds sps = get_uniform_surface_cover(in, get_squared(resolution));

  // which surface points support each sphere
  IMP_BASE_LARGE_UNORDERED_MAP<SphereIndex, SPIndexSet> supports;
  IMP_BASE_LARGE_UNORDERED_MAP<SphereIndex, double> radii;
  IMP_LOG_TERSE("Creating NN search structure." << std::endl);
  IMP_NEW(NearestNeighborD<3>, nns, (sps));

  IMP_LOG_TERSE("Searching for nearest neighbors." << std::endl);
  {
    std::ofstream cpout("/tmp/balls.bild");
    for (unsigned int i = 0; i < in.size(); ++i) {
      SphereIndex si(i);
      unsigned int nn = nns->get_nearest_neighbor(in[i].get_center());
      double r = get_distance(sps[nn], in[i].get_center());
      IMP_FOREACH(Vector3D v, sps) {
        IMP_INTERNAL_CHECK(get_distance(v, in[i].get_center()) > .9 * r,
                           "Bad nearest neighbor. Found one at "
                               << v << " with distance "
                               << get_distance(v, in[i].get_center())
                               << " as opposed to " << r << " from " << in[i]);
      }
      IMP::Ints support =
          nns->get_in_ball(in[i].get_center(), r + 1.0 / resolution);

      IMP_FOREACH(int i, support) { supports[si].insert(SPIndex(i)); }
      radii[si] = r + .5 / resolution;
      cpout << ".sphere " << in[i].get_center()[0] << " "
            << in[i].get_center()[1] << " " << in[i].get_center()[2] << " " << r
            << std::endl;
    }
  }

  for (unsigned int i = 0; i < in.size(); ++i) {
    SphereIndex si(i);
    IMP_FOREACH(Vector3D v, sps) {
      IMP_INTERNAL_CHECK(get_distance(v, in[i].get_center()) >=
                             radii.find(si)->second - 1.0 / resolution,
                         "Sphere too big at "
                             << in[i] << " with radius "
                             << radii.find(si)->second << " at surface point "
                             << v << " with distance "
                             << get_distance(v, in[i].get_center()));
    }
  }

  IMP_LOG_TERSE("Distributing support." << std::endl);
  // which spheres are supported by each point
  typedef std::pair<SPIndex, SphereIndexSet> SupportedPair;
  if (0) SupportedPair();  // suppress warning
  IMP_BASE_LARGE_UNORDERED_MAP<SPIndex, SphereIndexSet> supported;
  typedef std::pair<SphereIndex, SPIndexSet> SupportsPair;
  IMP_FOREACH(const SupportsPair & ps, supports) {
    IMP_FOREACH(SPIndex spi, ps.second) { supported[spi].insert(ps.first); }
  }

  for (unsigned int i = 0; i < sps.size(); ++i) {
    SPIndex spi(i);
    IMP_INTERNAL_CHECK(supported.find(spi) != supported.end(),
                       "Point is not supported");
  }

  IMP_LOG_TERSE("Generating output." << std::endl);
  Sphere3Ds ret;
  while (!supported.empty()) {
    IMP_USAGE_CHECK(!supports.empty(), "Out of spheres");
    SphereIndex max;
    double max_score = 0;
    IMP_FOREACH(const SupportsPair & sp, supports) {
      double score = sp.second.size();
      if (score > max_score) {
        max_score = score;
        max = sp.first;
      }
    }
    SPIndexSet max_supports = supports.find(max)->second;
    IMP_FOREACH(SPIndex spi, max_supports) {
      IMP_FOREACH(SphereIndex si, supported.find(spi)->second) {
        if (si != max) {
          supports[si].erase(spi);
          if (supports[si].empty()) supports.erase(si);
        }
      }
      supported.erase(spi);
    }
    supports.erase(max);
    ret.push_back(Sphere3D(in[max.get_index()].get_center(),
                           radii.find(max)->second - probe));
  }
  return ret;
}

IMPALGEBRA_END_NAMESPACE
