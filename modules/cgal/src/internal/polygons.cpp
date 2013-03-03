/**
 *  \file internal/cgal_knn.h
 *  \brief manipulation of text, and Interconversion between text and numbers
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
*/
// value in Lazy
#include <IMP/cgal/internal/polygons.h>
IMP_GCC_PRAGMA(diagnostic ignored "-Wuninitialized")
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/linear_least_squares_fitting_3.h>
#include <CGAL/Polygon_convex_decomposition_2.h>
#include <CGAL/ch_graham_andrew.h>
#include <CGAL/intersections.h>
#include <CGAL/Handle_hash_function.h>


typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef Kernel::Point_2 Point_2;
typedef Kernel::Point_3 Point_3;
typedef Kernel::Direction_3 Direction_3;
typedef Kernel::Vector_3 Vector_3;
typedef Kernel::Plane_3 Plane_3;
typedef Kernel::Segment_3 Segment_3;


IMPCGAL_BEGIN_INTERNAL_NAMESPACE


namespace {
  template <class To, class From>
  To tr(const From &v) {
    return To(v[0], v[1], v[2]);
  }

  struct PointLess {
    bool operator()(const Point_2 &a,
                    const Point_2 &b) const {
      if (a[0]==b[0]) return a[1] < b[1];
      else return a[0] < b[0];
    }
  };
}

algebra::Vector3Ds
get_intersection(const algebra::Vector3D &normal,
                 double d,
                 const algebra::BoundingBoxD<3> &bb) {
  typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
  typedef Kernel::Point_2 Point_2;
  typedef Kernel::Point_3 Point_3;
  typedef Kernel::Direction_3 Direction_3;
  typedef Kernel::Vector_3 Vector_3;
  typedef Kernel::Plane_3 Plane_3;
  typedef Kernel::Segment_3 Segment_3;
  Plane_3 plane(tr<Point_3>(normal*d), tr<Direction_3>(normal));
  std::cout << "plane " << plane << std::endl;
  algebra::Vector3Ds corners= algebra::get_vertices(bb);
  IntPairs edges= algebra::get_edges(bb);
  base::Vector<Point_2> intersections;
  for (unsigned int i=0; i< edges.size(); ++i) {
    Segment_3 s(tr<Point_3>(corners[edges[i].first]),
                tr<Point_3>(corners[edges[i].second]));
    std::cout << s << std::endl;
    CGAL::Object o=CGAL::intersection(plane, s);
    if (const Point_3* ip= CGAL::object_cast<Point_3>(&o)) {
      intersections.push_back(plane.to_2d(*ip));
    }
  }
  base::Vector<Point_2> ch;
  CGAL::ch_graham_andrew(intersections.begin(), intersections.end(),
                         std::back_inserter(ch));
  algebra::Vector3Ds ret;
  for (unsigned int i=0; i< ch.size(); ++i) {
    std::cout << "found " << ch[i] << " is "
              << plane.to_3d(ch[i]) << std::endl;
    ret.push_back(tr<algebra::Vector3D >(plane.to_3d(ch[i])));
  }
  return ret;
}



Ints
get_convex_polygons(const Ints &indexes,
                    const algebra::Vector3Ds &vertices) {
  if (indexes.size() <3) {
    IMP_THROW("Polygon must at least be a triangle", base::ValueException);
  }
  //std::cout << "Splitting polygon " << poly.size() << std::endl;
  typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
  //Kernel k;
  typedef Kernel::Point_2 Point_2;
  typedef Kernel::Point_3 Point_3;
  typedef Kernel::Plane_3 Plane_3;
  typedef CGAL::Partition_traits_2<Kernel>::Polygon_2 Polygon_2;
  base::Vector<Point_3> poly_3(indexes.size());
  for (unsigned int i=0; i< indexes.size(); ++i) {
    poly_3[i]= Point_3(vertices.at(indexes[i])[0], vertices.at(indexes[i])[1],
                       vertices.at(indexes[i])[2]);
  }
  // compute best fit plane
  Plane_3 plane;
  Point_3 centroid;
  CGAL::Dimension_tag<0> dt;
  CGAL::linear_least_squares_fitting_3(poly_3.begin(), poly_3.end(),
                                       plane, centroid,// k,
                                       dt);
  //plane= CGAL::object_cast<Plane_3>(out);
  // project points
  base::Vector<Point_2> points_2(indexes.size());
  std::map<Point_2, int, PointLess> index;
  for (unsigned int i=0; i< indexes.size(); ++i) {
    points_2[i]= plane.to_2d(poly_3[i]);
    index[points_2[i]]= i;
    //std::cout << "Got point " << points_2[i] << std::endl;
  }
  Polygon_2 poly2(points_2.begin(), points_2.end());
  bool reverse=false;
  if (!poly2.is_counterclockwise_oriented()) {
    poly2.reverse_orientation();
    reverse=true;
  }
  // decompose
  base::Vector<Polygon_2 > polys2;
  try {
    CGAL::approx_convex_partition_2(poly2.vertices_begin(),
                                    poly2.vertices_end(),
                                    std::back_inserter(polys2));
  } catch(...) {
    IMP_THROW("Polygon is not simple", base::ValueException);
  }
  //std::cout << "Got " << polys2.size() << std::endl;
  // unproject decomposition
  Ints ret;
  for (unsigned int i=0; i< polys2.size(); ++i) {
    for (Polygon_2::Vertex_iterator it = polys2[i].vertices_begin();
         it != polys2[i].vertices_end(); ++it) {
      IMP_INTERNAL_CHECK(index.find(*it) != index.end(),
                         "Can't find point " << *it);
      ret.push_back(index.find(*it)->second);
    }
    ret.push_back(-1);
    //std::cout << std::endl;
  }
  if (reverse) {
    ret.pop_back();
    std::reverse(ret.begin(), ret.end());
    ret.push_back(-1);
  }
  return ret;
}

IMPCGAL_END_INTERNAL_NAMESPACE
