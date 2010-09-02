/**
 *  \file writers.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/display/internal/utility.h>
#ifdef IMP_USE_CGAL
#include <IMP/cgal/internal/polygons.h>
#endif

IMPDISPLAY_BEGIN_INTERNAL_NAMESPACE
std::pair<std::vector<algebra::Vector3Ds>, algebra::Vector3D>
get_convex_polygons(const algebra::Vector3Ds &poly) {
#ifdef IMP_USE_CGAL
  return IMP::cgal::internal::get_convex_polygons(poly);
#else
  if (poly.size() <3) {
    IMP_THROW("Polygon must at least be a triangle", ValueException);
  }
  std::cout << "Splitting polygon " << poly.size() << std::endl;
  std::pair<std::vector<algebra::Vector3Ds>, algebra::Vector3D> ret;

  // ideally check if input is convex
  ret.first.push_back(poly);
  ret.second=algebra::get_vector_product(poly[0]-poly[1],
                                poly[2]-poly[1]).get_unit_vector();
  return ret;
#endif
}
IMPDISPLAY_END_INTERNAL_NAMESPACE
