/**
 *  \file Sphere3D.h   \brief Simple 3D sphere class.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */
#ifndef IMPALGEBRA_SPHERE_3D_H
#define IMPALGEBRA_SPHERE_3D_H

#include "Cylinder3D.h"
#include <cmath>
#include <IMP/constants.h>
#include "internal/cgal_predicates.h"

IMPALGEBRA_BEGIN_NAMESPACE

/** Represent a sphere using a point and a radius.
  */
class IMPALGEBRAEXPORT Sphere3D: public UninitializedDefault {
public:
  Sphere3D(){
#ifndef NDEBUG
    radius_= std::numeric_limits<double>::quiet_NaN();
#endif
  }
  Sphere3D(const Vector3D& center,double radius);
  double get_volume() const;
  double get_surface_area() const;
  double get_radius() const {
    IMP_check(!is_nan(radius_),
              "Attempt to use uninitialized sphere.",
              InvalidStateException);
    return radius_;
  }
  const Vector3D &get_center() const {return center_;}
  Cylinder3D get_bounding_cylinder() const {
    return Cylinder3D(Segment3D(get_center()-Vector3D(0.0,0.0,get_radius()),
                                get_center()+Vector3D(0.0,0.0,get_radius())),
                      get_radius());
  }
  //! Return true if this sphere contains the other one
  bool get_contains(const Sphere3D &o) const {
    double d= (get_center()-o.get_center()).get_magnitude();
    return (d+ o.get_radius() < get_radius());
  }

  //! Return true if the point is in or on the surface of the sphere
  /** \cgalpredicate
   */
  bool get_contains(const Vector3D &p) const {
#ifdef IMP_CGAL
    return internal::cgal_sphere_compare_inside(*this, p);
#else
    return ((p-center_).get_squared_magnitude() <= square(radius_));
#endif
  }

  void show(std::ostream &out=std::cout) const {
    out << "(" << spaces_io(center_) << ": " << get_radius()
        << ")";
  }
private:
  Vector3D center_;
  double radius_;
};

IMP_OUTPUT_OPERATOR(Sphere3D);

typedef std::vector<Sphere3D> Sphere3Ds;

//! Return the distance between the two spheres if they are disjoint
/** If they intersect, the distances are not meaningful.
    \relates Sphere3D
*/
inline double distance(const Sphere3D& a, const Sphere3D &b) {
  double d= (a.get_center()-b.get_center()).get_magnitude();
  return d - a.get_radius() - b.get_radius();
}


//! Return a sphere containing the listed spheres
/** \relates Sphere3D
    \note This method produces tighter bounding spheres if CGAL
    is used.
    \ingroup CGAL
 */
IMPALGEBRAEXPORT Sphere3D enclosing_sphere(const Sphere3Ds &ss);

//! Return the radius of a sphere with a given volume
/** \relates Sphere3D
 */
inline double ball_radius_from_volume(double volume) {
  return std::pow((.75/PI)*volume, .3333);
}

//! Return true if the two balls bounded by the two spheres interesect
/** \relates Sphere3D
 */
inline bool interiors_intersect(const Sphere3D &a, const Sphere3D &b) {
  double sr= a.get_radius() + b.get_radius();
  for (unsigned int i=0; i< 3; ++i) {
    double delta=std::abs(a.get_center()[i]- b.get_center()[i]);
    if (delta >= sr) return false;
  }
  return squared_distance(a.get_center(), b.get_center())
    < square(sr);
}

#ifndef SWIG

namespace internal {
  struct SphereSpacesIO
  {
    const Sphere3D &v_;
    SphereSpacesIO(const Sphere3D &v): v_(v){}
  };
  inline std::ostream &operator<<(std::ostream &out, const Sphere3D &s)
  {
    out << spaces_io(s.get_center()) << " " << s.get_radius();
    return out;
  }
}

//! Use this before outputing to a stream with spaces delimiting
/** std::cout << spaces_io(s);
    produces "1.0 2.0 3.0 4.0" where the radius is 4.0
    \relates Sphere3D
 */
inline internal::SphereSpacesIO spaces_io(const Sphere3D &v) {
  return internal::SphereSpacesIO(v);
}
#endif

#ifndef IMP_DOXYGEN
typedef std::pair<Sphere3D, Sphere3D> SpherePair;
#endif
IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_SPHERE_3D_H */
