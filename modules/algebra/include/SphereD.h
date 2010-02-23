/**
 *  \file SphereD.h   \brief Simple 3D sphere class.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */
#ifndef IMPALGEBRA_SPHERE_D_H
#define IMPALGEBRA_SPHERE_D_H

#include <cmath>
#include <IMP/constants.h>
#include "internal/cgal_predicates.h"
#include "BoundingBoxD.h"
#include "Vector3D.h"
#include "macros.h"

IMPALGEBRA_BEGIN_NAMESPACE

/** Represent a sphere in D-dimensions.
    \geometry
  */
template <unsigned int D>
class SphereD {
public:
  SphereD(){
#if IMP_BUILD < IMP_FAST
    radius_= std::numeric_limits<double>::quiet_NaN();
#endif
  }
  SphereD(const VectorD<D>& center,double radius):center_(center),
                                                  radius_(radius){}
  double get_radius() const {
    IMP_USAGE_CHECK(!is_nan(radius_),
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
    return ((p-center_).get_squared_magnitude() <= square(radius_));
  }
  IMP_SHOWABLE_INLINE({
      out << "(" << spaces_io(center_) << ": " << get_radius()
          << ")";
    })
private:
  VectorD<D> center_;
  double radius_;
};


IMP_VOLUME_GEOMETRY_METHODS_D(Sphere,
                              {
                                return PI * 4.0 * square(g.get_radius());
                              },
                              {
                                return PI * (4.0 / 3.0)
                                  * std::pow(g.get_radius(), 3.0);
                              },
                              return BoundingBoxD<D>(g.get_center())
                              +g.get_radius();
                            );

template <unsigned int D>
SphereD<D> get_unit_sphere_d() {
  return SphereD<D>(get_zero_vector_d<D>(), 1.0);
}

//! Return the distance between the two spheres if they are disjoint
/** If they intersect, the distances are not meaningful.
    \relatesalso SphereD
*/
template <unsigned int D>
inline double get_distance(const SphereD<D>& a, const SphereD<D> &b) {
  double d= (a.get_center()-b.get_center()).get_magnitude();
  return d - a.get_radius() - b.get_radius();
}

//! Return the power distance between the two spheres
/** The power distance is the square of the distance between the centers
    minus the sum of the square of the radii.
    \relatesalso SphereD
*/
template <unsigned int D>
inline double get_power_distance(const SphereD<D>& a, const SphereD<D> &b) {
  double d= (a.get_center()-b.get_center()).get_squared_magnitude();
  return d - square(a.get_radius()) - square(b.get_radius());
}


//! Return true if the two balls bounded by the two spheres interesect
/** \relatesalso SphereD
 */
template <unsigned int D>
inline bool get_interiors_intersect(const SphereD<D> &a, const SphereD<D> &b) {
  double sr= a.get_radius() + b.get_radius();
  for (unsigned int i=0; i< 3; ++i) {
    double delta=std::abs(a.get_center()[i]- b.get_center()[i]);
    if (delta >= sr) return false;
  }
  return get_squared_distance(a.get_center(), b.get_center())
    < square(sr);
}

#ifndef SWIG

namespace internal {
  template <unsigned int D>
  struct SphereSpacesIO
  {
    const SphereD<D> &v_;
    SphereSpacesIO(const SphereD<D> &v): v_(v){}
  };
  template <unsigned int D>
  inline std::ostream &operator<<(std::ostream &out, const SphereD<D> &s)
  {
    out << spaces_io(s.get_center()) << " " << s.get_radius();
    return out;
  }
}

//! Use this before outputing to a stream with spaces delimiting
/** std::cout << spaces_io(s);
    produces "1.0 2.0 3.0 4.0" where the radius is 4.0
    \relatesalso SphereD
 */
template <unsigned int D>
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

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_SPHERE_D_H */
