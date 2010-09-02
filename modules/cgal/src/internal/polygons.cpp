/**
 *  \file internal/cgal_knn.h
 *  \brief manipulation of text, and Interconversion between text and numbers
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#include <IMP/cgal/internal/polygons.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/linear_least_squares_fitting_3.h>
#include <CGAL/Polygon_convex_decomposition_2.h>


IMPCGAL_BEGIN_INTERNAL_NAMESPACE
std::pair<std::vector<std::vector<algebra::VectorD<3> > >, algebra::VectorD<3> >
get_convex_polygons(const std::vector<algebra::VectorD<3> > &poly) {
  if (poly.size() <3) {
    IMP_THROW("Polygon must at least be a triangle", ValueException);
  }
  std::cout << "Splitting polygon " << poly.size() << std::endl;
  std::pair<std::vector<algebra::Vector3Ds>, algebra::Vector3D> ret;
  typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
  //Kernel k;
  typedef Kernel::Point_2 Point_2;
  typedef Kernel::Point_3 Point_3;
  typedef Kernel::Plane_3 Plane_3;
  typedef CGAL::Partition_traits_2<Kernel>::Polygon_2 Polygon_2;
  std::vector<Point_3> poly_3(poly.size());
  for (unsigned int i=0; i< poly.size(); ++i) {
    poly_3[i]= Point_3(poly[i][0], poly[i][1], poly[i][2]);
  }
  // compute best fit plane
  Plane_3 plane;
  Point_3 centroid;
  CGAL::Dimension_tag<0> dt;
  CGAL::linear_least_squares_fitting_3(poly_3.begin(), poly_3.end(),
                                       plane, centroid,// k,
                                       dt);
  std::cout << "Plane is " << plane << std::endl;
  ret.second= algebra::Vector3D(plane.orthogonal_vector()[0],
                                plane.orthogonal_vector()[1],
                                plane.orthogonal_vector()[2]).get_unit_vector();
  //plane= CGAL::object_cast<Plane_3>(out);
  // project points
  std::vector<Point_2> points_2(poly.size());
  for (unsigned int i=0; i< poly.size(); ++i) {
    points_2[i]= plane.to_2d(poly_3[i]);
    std::cout << "Got point " << points_2[i] << std::endl;
  }
  Polygon_2 poly2(points_2.begin(), points_2.end());
  if (!poly2.is_counterclockwise_oriented()) {
    poly2.reverse_orientation();
    ret.second=-ret.second;
  }
  // decompose
  std::vector<Polygon_2 > polys2;
  try {
    CGAL::approx_convex_partition_2(poly2.vertices_begin(),
                                    poly2.vertices_end(),
                                    std::back_inserter(polys2));
  } catch(...) {
    IMP_THROW("Polygon is not simple", ValueException);
  }
  std::cout << "Got " << polys2.size() << std::endl;
  // unproject decomposition
  for (unsigned int i=0; i< polys2.size(); ++i) {
    ret.first.push_back(algebra::Vector3Ds());
    for (Polygon_2::Vertex_iterator it = polys2[i].vertices_begin();
         it != polys2[i].vertices_end(); ++it) {
      Point_3 pt= plane.to_3d(*it);
      std::cout << pt << ", ";
      ret.first.back().push_back(algebra::Vector3D(pt[0], pt[1], pt[2]));
    }
    std::cout << std::endl;
  }
  return ret;
}


IMPCGAL_END_INTERNAL_NAMESPACE
