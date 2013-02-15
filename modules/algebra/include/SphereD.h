/**
 *  \file IMP/algebra/SphereD.h   \brief Simple 3D sphere class.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_SPHERE_D_H
#define IMPALGEBRA_SPHERE_D_H

#include "algebra_macros.h"
#include "constants.h"
#include "BoundingBoxD.h"
#include "VectorD.h"
#include "utility.h"
#include "GeometricPrimitiveD.h"
#include <cmath>

IMPALGEBRA_BEGIN_NAMESPACE

/** Represent a sphere in D-dimensions.
    \geometry
  */
template <int D>
class SphereD: public GeometricPrimitiveD<D> {
public:
  SphereD(){
#if IMP_HAS_CHECKS >= IMP_USAGE
    radius_= std::numeric_limits<double>::quiet_NaN();
#endif
  }
  SphereD(const VectorD<D>& center,double radius):center_(center),
                                                  radius_(radius){
    IMP_USAGE_CHECK(radius>=0, "Radius can't be negative");
  }
  double get_radius() const {
    IMP_USAGE_CHECK(!base::isnan(radius_),
              "Attempt to use uninitialized sphere.");
    return radius_;
  }
  const VectorD<D> &get_center() const {return center_;}
  //! Return true if this sphere contains the other one
  bool get_contains(const SphereD<D> &o) const {
    double d= (get_center()-o.get_center()).get_magnitude();
    return (d+ o.get_radius() < get_radius());
  }

  //! Return true if the point is in or on the surface of the sphere
  /**
   */
  bool get_contains(const VectorD<D> &p) const {
    return ((p-center_).get_squared_magnitude() <= get_squared(radius_));
  }
  IMP_SHOWABLE_INLINE(SphereD, {
      out << "(" << spaces_io(center_) << ": " << get_radius()
          << ")";
    });
#ifndef IMP_DOXYGEN
#ifndef SWIG
  VectorD<D> &_access_center() {
    return center_;
  }
  void _set_radius(double d) {
    radius_=d;
  }
  double &operator[](unsigned int i) {
    IMP_USAGE_CHECK(i<D+1, "Out of range");
    if (i <D) {
      return center_[i];
    } else {
      return radius_;
    }
  }
  double operator[](unsigned int i) const {
    IMP_USAGE_CHECK(i<D+1, "Out of range");
    if (i <D) {
      return center_[i];
    } else {
      return radius_;
    }
  }
#endif
#endif
  unsigned int get_dimension() const {
    return center_.get_dimension();
  }
private:
  VectorD<D> center_;
  double radius_;
};


IMP_VOLUME_GEOMETRY_METHODS_D(Sphere, sphere,
                              {
                                return PI * 4.0 * get_squared(g.get_radius());
                              },
                              {
                                return PI * (4.0 / 3.0)
                                  * std::pow(g.get_radius(), 3.0);
                              },
                              return BoundingBoxD<D>(g.get_center())
                              +g.get_radius();
                            );

template <unsigned int D>
inline SphereD<D> get_unit_sphere_d() {
  return SphereD<D>(get_zero_vector_d<D>(), 1.0);
}

inline SphereD<-1> get_unit_sphere_kd(unsigned int d) {
  return SphereD<-1>(get_zero_vector_kd(d), 1.0);
}

//! Return the distance between the two spheres if they are disjoint
/** If they intersect, the distances are not meaningful.
    \relatesalso SphereD
*/
template <int D>
inline double get_distance(const SphereD<D>& a, const SphereD<D> &b) {
  double d= (a.get_center()-b.get_center()).get_magnitude();
  return d - a.get_radius() - b.get_radius();
}

//! Return the power distance between the two spheres
/** The power distance is the square of the distance between the centers
    minus the sum of the square of the radii.
    \relatesalso SphereD
*/
template <int D>
inline double get_power_distance(const SphereD<D>& a, const SphereD<D> &b) {
  double d= (a.get_center()-b.get_center()).get_squared_magnitude();
  return d - square(a.get_radius()) - square(b.get_radius());
}


//! Return true if the two balls bounded by the two spheres interesect
/** \relatesalso SphereD
 */
template <int D>
inline bool get_interiors_intersect(const SphereD<D> &a, const SphereD<D> &b) {
  double sr= a.get_radius() + b.get_radius();
  for (unsigned int i=0; i< 3; ++i) {
    double delta=std::abs(a.get_center()[i]- b.get_center()[i]);
    if (delta >= sr) return false;
  }
  return get_squared_distance(a.get_center(), b.get_center())
    < get_squared(sr);
}

#if !defined(SWIG) && !defined(IMP_DOXYGEN)

namespace internal {
  template <int D>
  struct SphereSpacesIO
  {
    const SphereD<D> &v_;
    SphereSpacesIO(const SphereD<D> &v): v_(v){}
  };
  template <int D>
  inline std::ostream &operator<<(std::ostream &out, const SphereSpacesIO<D> &s)
  {
    for (unsigned int i=0; i< s.v_.get_center().get_dimension(); ++i) {
      out << s.v_.get_center()[i] << " ";
    }
    out << s.v_.get_radius();
    return out;
  }
}

//! Use this before outputing to a stream with spaces delimiting
/** std::cout << spaces_io(s);
    produces "1.0 2.0 3.0 4.0" where the radius is 4.0
    \relatesalso SphereD
 */
template <int D>
inline internal::SphereSpacesIO<D> spaces_io(const SphereD<D> &v) {
  return internal::SphereSpacesIO<D>(v);
}
#endif

#ifdef IMP_DOXYGEN
//! Compute the bounding box of any geometric object
template <class Geometry>
BoundingBoxD<3> get_bounding_box(const Geometry &);
//! Compute the surface area of any volumetric object
template <class Geometry>
double get_surface_area(const Geometry &);
//! Compute the volume of any volumetric object
template <class Geometry>
double get_volume(const Geometry &);
//! Compute the area of any surface object
template <class Geometry>
double get_area(const Geometry &);

#endif
template <int D>
VectorD<D> get_vector_d_geometry(const SphereD<D> &s) {
  return s.get_center();
}

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_SPHERE_D_H */
