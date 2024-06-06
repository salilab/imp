/**
 *  \file cgal_predicates.h
 *  \brief predicates implemented using CGAL
 *  Copyright 2007-2024 IMP Inventors. All rights reserved.
 */

#ifndef IMPALGEBRA_INTERNAL_VECTOR_GENERATORS_H
#define IMPALGEBRA_INTERNAL_VECTOR_GENERATORS_H
#include <IMP/algebra/algebra_config.h>
#include "../VectorD.h"
#include "../SphereD.h"
#include "../SphericalVector3D.h"
#include "../UnitSimplexD.h"
#include "../utility.h"
#include "utility.h"
#include <IMP/random_utils.h>
#ifdef IMP_ALGEBRA_USE_IMP_CGAL
#include <IMP/cgal/internal/sphere_cover.h>
#endif
#include <limits>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <boost/random/exponential_distribution.hpp>
#include <boost/random/variate_generator.hpp>

IMPALGEBRA_BEGIN_INTERNAL_NAMESPACE
template <int D>
inline VectorD<D> get_random_vector_in(const BoundingBoxD<D> &bb) {
  VectorD<D> ret = bb.get_corner(0);  // some appropriate vector
  for (unsigned int i = 0; i < bb.get_dimension(); ++i) {
    ::boost::random::uniform_real_distribution<> rand(bb.get_corner(0)[i],
                                                      bb.get_corner(1)[i]);
    ret[i] = rand(random_number_generator);
  }
  return ret;
}

template <int D>
inline VectorD<D> get_random_vector_on(const SphereD<D> &s) {
  BoundingBoxD<D> bb = get_unit_bounding_box_d<D>();
  do {
    VectorD<D> pt = internal::get_random_vector_in(bb);
    double r2 = pt.get_squared_magnitude();
    if (r2 < 1 && r2 > .1) {
      VectorD<D> udiff = pt.get_unit_vector();
      return s.get_center() + udiff * s.get_radius();
    }
  } while (true);
}

// sillyness due to lack of generic api for creating kd objects
inline VectorD<-1> get_random_vector_on(const SphereD<-1> &s) {
  BoundingBoxD<-1> bb = get_unit_bounding_box_kd(s.get_dimension());
  do {
    VectorD<-1> pt = get_random_vector_in(bb);
    double r2 = pt.get_squared_magnitude();
    if (r2 < 1 && r2 > .01) {
      VectorD<-1> udiff = pt.get_unit_vector();
      return s.get_center() + udiff * s.get_radius();
    }
  } while (true);
}

inline VectorD<2> get_random_vector_on(const SphereD<2> &s) {
  ::boost::random::uniform_real_distribution<> rand(0, 2 * PI);
  double angle = rand(random_number_generator);
  VectorD<2> ret(s.get_radius() * sin(angle), s.get_radius() * cos(angle));
  return ret + s.get_center();
}

//! returns a random vector on a sphere of radius 1
//! with implementation optimized for the 3D unit vector case
inline VectorD<3> get_random_vector_on_unit_sphere() {
  //  ::boost::random::uniform_real_distribution<> rand(-1, 1);
  do {
    //    double x1 = rand(random_number_generator);
    //    double x2 = rand(random_number_generator);
    double x1 = IMP::get_random_double_uniform(-1,1);
    double x2 = IMP::get_random_double_uniform(-1,1);
    double ssq = get_squared(x1) + get_squared(x2);
    if (ssq <= 1) { // = (x1,x2) is a random point sampled uniformly inside the unit circle
      VectorD<3> ret;
      double sq = std::sqrt(1 - ssq); // x3 s.t. (x1,x2,x3) is on the unit sphere
      ret[0] = 2 * x1 * sq; // =  4 (1-x1^2-x2^2) x1^2  =  4 x1^2 - 4 x1^4 - 4 x1^2 x2^2
      ret[1] = 2 * x2 * sq; // =  4 (1-x1^2-x2^2) x2^2  =  4 x2^2 - 4 x2^4 - 4 x1^2 x2^2
      ret[2] = 1 - 2*ssq;   // =  1 - 4 x1^2 - 4 x2^2 + 4 x1^4 + 8 x1^2 x2^2 + 4 x2^4
      return ret;
    }
  } while (true);
}

//! returns a random vector on a unit simplex
template <int D>
inline VectorD<D> get_random_vector_on(const UnitSimplexD<D> &s) {
  boost::exponential_distribution<> dist(1.0);
  boost::variate_generator<RandomNumberGenerator&,
                           boost::exponential_distribution<double> >
      randexp(random_number_generator, dist);
  int d = s.get_dimension();
  VectorD<D> p = get_zero_vector_kd<D>(d);
  double psum = 0;
  for (int i = 0; i < d; ++i) {
    p[i] = randexp();
    psum += p[i];
  }
  p /= psum;
  return p;
}

//! returns a random vector on the surface of the sphere s in 3D
//! with implementation optimized for the 3D case
inline VectorD<3> get_random_vector_on(const SphereD<3> &s) {
  return s.get_center() + s.get_radius() * get_random_vector_on_unit_sphere();
}

/*inline VectorD<3> get_random_vector_on(const SphereD<3> &s) {
  double cur_radius2=get_squared(s.get_radius());
  Floats up(s.get_center().get_dimension());
  for (unsigned int i=s.get_dimension()-1; i>0; --i) {
    double r= std::sqrt(cur_radius2);
    ::boost::random::uniform_real_distribution<> rand(-r, r);
    up[i]= rand(random_number_generator);
    // radius of circle
    cur_radius2= cur_radius2-get_squared(up[i]);
  }
  ::boost::random::uniform_int_distribution<> rand(0, 1);
  double x= std::sqrt(cur_radius2);
  if (rand(random_number_generator)) {
    x=-x;
  }
  up[0]=x;

  IMP_INTERNAL_CHECK(std::abs(VectorD<3>(up.begin(),
                                         up.end()).get_magnitude()
                              -s.get_radius()) < .1,
                     "Error generating vector on sphere: "
                     << VectorD<3>(up.begin(), up.end())
                     << " for " << s.get_radius());
  //IMP_LOG_VERBOSE( "Random vector on sphere is " << up << std::endl);

  return s.get_center()+ VectorD<3>(up.begin(), up.end());
  }*/

/*inline VectorKD get_random_vector_on(const SphereKD &s) {
  if (s.get_dimension()==2) {
    Vector2D pt
    = get_random_vector_on(Sphere2D(Vector2D(s.get_center().coordinates_begin(),
    s.get_center().coordinates_end()),
                                               s.get_radius()));
    return pt;
  }
  }*/

template <int D>
inline Vector<VectorD<D> > native_uniform_cover_unit_sphere(
    unsigned int d, unsigned int n, bool ALL) {
  static_assert(D != 3, "VectorD wrong size");
  Vector<VectorD<D> > ret(n);
  for (unsigned int i = 0; i < std::min(d, n / (ALL ? 2 : 1)); ++i) {
    VectorD<D> v = get_basis_vector_kd(d, i);
    if (ALL) {
      ret[2 * i] = v;
      ret[2 * i + 1] = -v;
    } else {
      ret[i] = v;
    }
  }
  for (unsigned int i = (ALL ? 2 * d : d); i < n; ++i) {
    VectorD<D> v = internal::get_random_vector_on(get_unit_sphere_kd(d));
    if (!ALL && v[d - 1] <= 0) v = -v;
    ret[i] = v;
  }
  return ret;
}

/*If all is true, cover the whole sphere.
 */
template <int D>
inline Vector<VectorD<D> > uniform_cover_sphere(unsigned int n,
                                                      const VectorD<D> &center,
                                                      double radius, bool ALL) {
  Vector<VectorD<D> > ret =
      native_uniform_cover_unit_sphere<D>(center.get_dimension(), n, ALL);
  for (unsigned int i = 0; i < ret.size(); ++i) {
    if (!ALL && ret[i][center.get_dimension() - 1] < 0) {
      ret[i] = -radius * ret[i] + center;
    } else {
      ret[i] = radius * ret[i] + center;
    }
  }
  return ret;
}

/*If all is true, cover the whole sphere.
 */
inline Vector<VectorD<4> > uniform_cover_sphere(unsigned int n,
                                                      const VectorD<4> &center,
                                                      double radius, bool ALL) {
  Vector<VectorD<4> > ret =
      native_uniform_cover_unit_sphere<4>(center.get_dimension(), n, ALL);
#ifdef IMP_ALGEBRA_USE_IMP_CGAL
  IMP::cgal::internal::refine_unit_sphere_cover_4d(ret, ALL);
#endif
  for (unsigned int i = 0; i < ret.size(); ++i) {
    if (!ALL && ret[i][4 - 1] < 0) {
      ret[i] = -radius * ret[i] + center;
    } else {
      ret[i] = radius * ret[i] + center;
    }
  }

  return ret;
}

//! returns a uniform cover of the sphere (center,r) with N points
//! ALL indicates whether to cover the whole sphere or only one hemisphere
inline Vector3Ds uniform_cover_sphere(unsigned int N, const Vector3D &center,
                                      double r, bool ALL) {
  if (N == 0 ) {
    return Vector3Ds();
  }
  if (N == 1) {
    Vector3D ret = center;
    ret[0] += r;
    return Vector3Ds(1, ret);
  }

  Vector3Ds ret(N);

  double f = 1;
  if (!ALL) {
    f = 2.0;
  }
  double opsi(1000000.0);
  for (unsigned long k = 1; k <= N; ++k) {
    double h = 2.0 * (k - 1.0) / (f * N - 1) - 1.0;
    double theta = std::acos(h);
    double psi;
    if (k == 1 || (ALL && k == N)) {
      psi = 0;
    } else {
      IMP_USAGE_CHECK(opsi<10000.0,
                      "opsi should have been initialized by now");
      psi = opsi + 3.6 / std::sqrt(f * (1.0 - h * h));
      int div = static_cast<int>(psi / (2.0 * PI));
      psi -= div * 2.0 * PI;
    }
    SphericalVector3D v(r, theta, psi);
    opsi = psi;
    if (!ALL) {
      ret[k - 1] = -v.get_cartesian_coordinates() + center;
    } else {
      ret[k - 1] = v.get_cartesian_coordinates() + center;
    }
  }
  return ret;
}

inline Vector2Ds uniform_cover_sphere(unsigned int N, const Vector2D &center,
                                      double r, bool ALL) {
  Vector2Ds ret(N);
  for (unsigned int i = 0; i < N; ++i) {
    double f;
    if (ALL) {
      f = static_cast<double>(i) / (N + 1);
    } else {
      f = static_cast<double>(i) / (2 * N + 2);
    }
    ret[i] = center + r * Vector2D(sin(f), cos(f));
  }
  return ret;
}

template <int D>
struct RandomVectorOnBB {
  static VectorD<D> get(BoundingBoxD<D> bb) {
    IMP_USAGE_CHECK(D > 0, "Does not work in runtime D yet");
    boost::scoped_array<double> areas(new double[bb.get_dimension() * 2]);
    VectorD<D> lb = bb.get_corner(0);
    VectorD<D> ub = bb.get_corner(1);
    for (unsigned int i = 0; i < bb.get_dimension(); ++i) {
      areas[i] = 1;
      for (unsigned int j = 1; j < bb.get_dimension(); ++j) {
        areas[i] *=
            ub[(i + j) % bb.get_dimension()] - lb[(i + j) % bb.get_dimension()];
      }
      if (i != 0) {
        areas[i] += areas[i - 1];
      }
    }
    for (unsigned int i = 0; i < bb.get_dimension(); ++i) {
      areas[bb.get_dimension() + i] = areas[bb.get_dimension() - 1] + areas[i];
    }
    /*for (unsigned int i=0; i< D*2; ++i) {
      std::cout << areas[i] << " ";
      }*/
    ::boost::random::uniform_real_distribution<> rand(
                                  0, areas[2 * bb.get_dimension() - 1]);
    double a = rand(random_number_generator);
    // std::cout << ": " << a << std::endl;
    unsigned int side;
    for (side = 0; side < 2 * bb.get_dimension(); ++side) {
      if (areas[side] > a) break;
    }
    unsigned int coord =
        (side >= bb.get_dimension() ? side - bb.get_dimension() : side);
    Floats fmin(bb.get_dimension() - 1), fmax(bb.get_dimension() - 1);
    for (unsigned int i = 1; i < bb.get_dimension(); ++i) {
      fmin[i - 1] = 0;
      fmax[i - 1] = ub[(coord + i) % bb.get_dimension()] -
                    lb[(coord + i) % bb.get_dimension()];
    }
    VectorD<internal::DMinus1<D>::D> vfmin(fmin.begin(), fmin.end()),
        vfmax(fmax.begin(), fmax.end());
    VectorD<internal::DMinus1<D>::D> sv =
        algebra::internal::get_random_vector_in(
            BoundingBoxD<internal::DMinus1<D>::D>(vfmin, vfmax));

    Floats ret(bb.get_dimension());
    // std::cout << "Side is " << side << std::endl;
    if (side >= bb.get_dimension()) {
      std::copy(ub.begin(), ub.end(), ret.begin());
      for (unsigned int i = 1; i < bb.get_dimension(); ++i) {
        ret[(coord + i) % bb.get_dimension()] -= sv[i - 1];
      }
    } else {
      std::copy(lb.begin(), lb.end(), ret.begin());
      for (unsigned int i = 1; i < bb.get_dimension(); ++i) {
        ret[(coord + i) % bb.get_dimension()] += sv[i - 1];
      }
    }

    return VectorD<D>(ret.begin(), ret.end());
  }
};

template <>
struct RandomVectorOnBB<1> {
  static VectorD<1> get(BoundingBoxD<1> bb) {
    ::boost::random::uniform_int_distribution<> rand(0, 1);
    return bb.get_corner(rand(random_number_generator));
  }
};

IMPALGEBRA_END_INTERNAL_NAMESPACE

#endif /* IMPALGEBRA_INTERNAL_VECTOR_GENERATORS_H */
