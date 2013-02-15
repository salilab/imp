/**
 *  \file vector_generators.cpp
 *  \brief Support for rigid bodies.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/algebra/vector_generators.h>
#include <IMP/base/log_macros.h>

IMPALGEBRA_BEGIN_NAMESPACE

/** Generate a random vector in a circle
    with uniform density with respect to the area of the circle

    @param s a 2D sphere (circle)

    \relatealso VectorD
    \relatesalso SphereD
*/
VectorD<2>
get_random_vector_in
(const SphereD<2> &s) {
  ::boost::uniform_real<> rand(0, 1);
  double angle =
    2 * PI * rand(base::random_number_generator);
  // sample the radius uniformly with respect to the circle area PI * R^2
  double R2 = std::pow(s.get_radius(), 2) ;
  double r=
    std::sqrt( R2 * rand(base::random_number_generator) );
  VectorD<2> ret(r * sin(angle),
                 r * cos(angle));
  return ret + s.get_center();
}


//! Generate a random vector in a cylinder with uniform density
/** \relatesalso VectorD
    \relatesalso Cylinder3D
 */
Vector3D
get_random_vector_in
(const Cylinder3D &c)
{
  ::boost::uniform_real<> rand(0, 1);
  // relative height and radius are between 0 (0%) and 1 (100%):
  double relative_h =
    rand(base::random_number_generator);
  // sqrt[rand(0,1)] is uniform with respect to the circle area PI*r^2
  double relative_r =
    std::sqrt( rand(base::random_number_generator) );
  double angle =
    2 * PI * rand(base::random_number_generator);
  return c.get_inner_point_at(relative_h, relative_r, angle);
}


Vector3Ds get_uniform_surface_cover(const Cylinder3D &cyl,
                                    int n) {
  if (n==0) {
    return Vector3Ds();
  }
  double c= 2*PI*cyl.get_radius();
  double h= cyl.get_segment().get_length();
  // h*c= num_points*l*l
  // l= sqrt(h*c/n)
  double l= std::sqrt(h*c/n);
  int cn= static_cast<int>(std::ceil(c/l));
  int hn= static_cast<int>(std::ceil(h/l));
  while ((hn-1)*cn >= n || hn*(cn-1) >=n) {
    if ((hn-1)*cn >= n && hn > cn) --hn;
    else --cn;
  }
  /*std::cout << "Asked for " << n << " got " << cn << "x"<< hn
    << "= " << cn*hn << std::endl;*/
  return get_grid_surface_cover(cyl, hn, cn);
}

namespace {
  // from 0,0,0 to 0,0,1 with radius 1
  // nc = number of points on each cycle
  // nh = number of cycles along the cylinder edge
  // scaling - in each coordinate - (X,Y) = cirlcle (Z) = cylinder length
  Vector3Ds grid_unit_cylinder(unsigned int nc, unsigned int nh,
                               const Vector3D &scaling,
                               const Transformation3D &transform) {
    if (nh==0) {
      return Vector3Ds();
    }
    Vector3Ds ret;
    for (unsigned int i=0; i< nh; ++i) {
      double h;
      if (nh==1) {
        h= .5;
      } else {
        h= static_cast<double>(i)/(nh-1);
      }
      for (unsigned int j=0; j< nc; ++j) {
        double a= 2*PI*static_cast<double>(j)/nc;
        Vector3D pt(scaling[0]*sin(a), scaling[1]*cos(a), scaling[2]*h);
        Vector3D tpt= transform.get_transformed(pt);
        ret.push_back(tpt);
      }
    }
    return ret;
  }
}

Vector3Ds get_grid_surface_cover(const Cylinder3D &cyl,
                                 int number_of_cycles,
                                 int number_of_points_on_cycle){
  Vector3D scale(cyl.get_radius(), cyl.get_radius(),
                 cyl.get_segment().get_length());
  Rotation3D rot
    = get_rotation_taking_first_to_second(Vector3D(0,0,1),
                                          cyl.get_segment().get_point(1)
                                          - cyl.get_segment().get_point(0));
  Transformation3D tr(rot, cyl.get_segment().get_point(0));
  return grid_unit_cylinder(number_of_points_on_cycle, number_of_cycles,
                            scale, tr);
}


Vector3Ds get_uniform_surface_cover(const SpherePatch3D &sph,
                                    unsigned int number_of_points) {
  Vector3Ds points;
  while (points.size() < number_of_points) {
    Vector3D rp = get_random_vector_on(sph.get_sphere());
#if IMP_HAS_CHECKS >= IMP_INTERNAL
    double r2
      = (rp-sph.get_sphere().get_center())
      .get_squared_magnitude();
    IMP_INTERNAL_CHECK(std::abs(r2- get_squared(sph.get_sphere().get_radius()))
                       < .05 *r2,
               "Bad point on sphere " << r2
               << " " << get_squared(sph.get_sphere().get_radius())
                       << std::endl);
#endif
    if (sph.get_contains(rp)) {
      points.push_back(rp);
    }
  }
  return points ;
}

Vector3Ds get_uniform_surface_cover(const Cone3D &cone,
                        unsigned int number_of_points) {
 Vector3Ds points;
 Vector3D sph_p;
 Sphere3D sph(cone.get_tip(), std::sqrt(get_squared(cone.get_radius())
                                        +get_squared(cone.get_height())));
 while (points.size() < number_of_points) {
   sph_p=get_random_vector_in(sph);
   if (cone.get_contains(sph_p)) {
     points.push_back(sph_p);
   }
 }
 return points;
}



Vector3Ds get_random_chain(unsigned int n, double r,
                           const Vector3D &start,
                           const Sphere3Ds &obstacles) {
  IMP_USAGE_CHECK(r>.00001,
            "If r is too small, it won't succeed in placing the spheres");
  unsigned int max_failures=30;
  Vector3Ds ret;
  base::Vector<unsigned int> failures;
  ret.push_back(start);
  failures.push_back(0);
  while (ret.size() != n) {
    if (ret.empty()) {
      IMP_FAILURE("Cannot place first random point");
    }
    if (failures.back() > max_failures) {
      IMP_LOG_VERBOSE( "Popping " << ret.back() << std::endl);
      ret.pop_back();
      failures.pop_back();
    }
    Vector3D v= get_random_vector_on(Sphere3D(ret.back(), 2*r));
    IMP_LOG_VERBOSE( "Trying " << v << " (" << ret.size() << ")"
            << std::endl);
    Sphere3D cb(v, r); // some slack
    bool bad=false;
    for (unsigned int i=0; i< ret.size()-1; ++i) {
      if (get_interiors_intersect(cb, Sphere3D(ret[i], r))) {
        bad=true;
        break;
      }
    }
    if (!bad) {
      for (unsigned int i=0; i< obstacles.size(); ++i) {
        if (get_interiors_intersect(obstacles[i], cb)) {
          bad=true;
          break;
        }
      }
    }
    if (bad) {
      ++failures.back();
    } else {
      ret.push_back(v);
      failures.push_back(0);
    }
  }
  return ret;
}


IMPALGEBRA_END_NAMESPACE
