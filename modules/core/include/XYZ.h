/**
 *  \file IMP/core/XYZ.h     \brief Simple xyz decorator.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_XY_Z_H
#define IMPCORE_XY_Z_H

#include <IMP/core/core_config.h>
#include <IMP/decorator_macros.h>
#include <IMP/Decorator.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/Transformation3D.h>
#include <vector>
#include <limits>

IMPCORE_BEGIN_NAMESPACE

//! A decorator for a particle with x,y,z coordinates.
/** \inlineimage{xyz.png, 50} Using the decorator one can
    get and set coordinates and modify derivatives.

    \ingroup helper
    \ingroup decorators
    \include XYZ_Decorator.py
    \see XYZR
 */
class IMPCOREEXPORT XYZ : public Decorator {
  static void do_setup_particle(
      kernel::Model *m, kernel::ParticleIndex pi,
      // This method and the next one need to take a vector (not a ref)
      // as otherwise, you can pass the vector from one and use it to
      // create another. But this would resize the vector and so invalidate
      // the passed reference. Ick.
      const algebra::Vector3D v = algebra::Vector3D(0, 0, 0)) {
    m->add_attribute(get_coordinate_key(0), pi, v[0]);
    m->add_attribute(get_coordinate_key(1), pi, v[1]);
    m->add_attribute(get_coordinate_key(2), pi, v[2]);
  }

 public:
  static FloatKey get_coordinate_key(unsigned int i) {
    IMP_USAGE_CHECK(i < 3, "Out of range coordinate");
    return IMP::internal::xyzr_keys[i];
  }

  IMP_DECORATOR_METHODS(XYZ, Decorator);
  /** Setup the particle with unspecified coordinates. */
  IMP_DECORATOR_SETUP_0(XYZ);
  IMP_DECORATOR_SETUP_1(XYZ, algebra::Vector3D, v);

  IMP_DECORATOR_GET_SET(x, get_coordinate_key(0), Float, Float);
  IMP_DECORATOR_GET_SET(y, get_coordinate_key(1), Float, Float);
  IMP_DECORATOR_GET_SET(z, get_coordinate_key(2), Float, Float);
  //! set the ith coordinate
  void set_coordinate(unsigned int i, Float v) {
    get_model()->get_sphere(get_particle_index())[i] = v;
  }
  //! set all coordinates from a vector
  void set_coordinates(const algebra::Vector3D &v) {
    get_model()->get_sphere(get_particle_index())[0] = v[0];
    get_model()->get_sphere(get_particle_index())[1] = v[1];
    get_model()->get_sphere(get_particle_index())[2] = v[2];
  }

  //! Get the ith coordinate
  Float get_coordinate(int i) const {
    return get_model()->get_sphere(get_particle_index())[i];
  }
  //! Get the ith coordinate derivative
  Float get_derivative(int i) const { return get_derivatives()[i]; }
  //! Add something to the derivative of the ith coordinate
  void add_to_derivative(int i, Float v, DerivativeAccumulator &d) {
    get_particle()->add_to_derivative(get_coordinate_key(i), v, d);
  }
  //! Add something to the derivative of the coordinates
  void add_to_derivatives(const algebra::Vector3D &v,
                          DerivativeAccumulator &d) {
    get_model()->add_to_coordinate_derivatives(get_particle_index(), v, d);
  }
  //! Get whether the coordinates are optimized
  /** \return true only if all of them are optimized.
    */
  bool get_coordinates_are_optimized() const {
    return get_particle()->get_is_optimized(get_coordinate_key(0)) &&
           get_particle()->get_is_optimized(get_coordinate_key(1)) &&
           get_particle()->get_is_optimized(get_coordinate_key(2));
  }
  //! Set whether the coordinates are optimized
  void set_coordinates_are_optimized(bool tf) const {
    get_particle()->set_is_optimized(get_coordinate_key(0), tf);
    get_particle()->set_is_optimized(get_coordinate_key(1), tf);
    get_particle()->set_is_optimized(get_coordinate_key(2), tf);
  }

  //! Get the vector from this particle to another
  algebra::Vector3D get_vector_to(const XYZ &b) const {
    return b.get_coordinates() - get_coordinates();
    ;
  }

  //! Convert it to a vector.
  /** Somewhat suspect based on wanting a Point/Vector differentiation
      but we don't have points */
  const algebra::Vector3D &get_coordinates() const {
    return get_model()->get_sphere(get_particle_index()).get_center();
  }

  //! Get the vector of derivatives.
  /** Somewhat suspect based on wanting a Point/Vector differentiation
      but we don't have points */
  algebra::Vector3D get_derivatives() const {
    return get_model()->get_coordinate_derivatives(get_particle_index());
  }

  static bool get_is_setup(kernel::Model *m, kernel::ParticleIndex pi) {
    return m->get_has_attribute(get_coordinate_key(2), pi);
  }

  //! Get a vector containing the keys for x,y,z
  /** This is quite handy for initializing movers and things.
   */
  static const FloatKeys &get_xyz_keys();
};

/** \genericgeometry */
inline void set_vector_geometry(XYZ d, const algebra::Vector3D &v) {
  d.set_coordinates(v);
}
/** \genericgeometry */
inline const algebra::Vector3D get_vector_geometry(XYZ d) {
  return d.get_coordinates();
}

IMPCORE_END_NAMESPACE

#ifndef SWIG
// swig doesn't like having the overloads in different namespaces
// it will do the conversion implicitly anyway
IMPKERNEL_BEGIN_NAMESPACE

/** \genericgeometry */
inline const algebra::Vector3D get_vector_geometry(Particle *p) {
  return core::XYZ(p).get_coordinates();
}
/** \genericgeometry */
inline void set_vector_geometry(Particle *p, const algebra::Vector3D &v) {
  core::XYZ(p).set_coordinates(v);
}

inline const algebra::Vector3D get_vector_geometry(Decorator d) {
  return core::XYZ(d).get_coordinates();
}
/** \genericgeometry */
inline void set_vector_geometry(Decorator d, const algebra::Vector3D &v) {
  core::XYZ(d).set_coordinates(v);
}

IMPKERNEL_END_NAMESPACE
IMPBASE_BEGIN_NAMESPACE
inline const algebra::Vector3D get_vector_geometry(
    base::WeakPointer<kernel::Particle> d) {
  return core::XYZ(d).get_coordinates();
}
/** \genericgeometry */
inline void set_vector_geometry(base::WeakPointer<kernel::Particle> d,
                                const algebra::Vector3D &v) {
  core::XYZ(d).set_coordinates(v);
}
inline const algebra::Vector3D get_vector_geometry(
    base::Pointer<kernel::Particle> d) {
  return core::XYZ(d).get_coordinates();
}
/** \genericgeometry */
inline void set_vector_geometry(base::Pointer<kernel::Particle> d,
                                const algebra::Vector3D &v) {
  core::XYZ(d).set_coordinates(v);
}
IMPBASE_END_NAMESPACE

#endif

IMPCORE_BEGIN_NAMESPACE

//! Compute the distance between a pair of particles
/** compute the ditance between the x,y,z coordinates of a and b
    \ingroup helper
    See XYZ
 */
inline double get_distance(XYZ a, XYZ b) {
  return algebra::get_distance(a.get_coordinates(), b.get_coordinates());
}

//! Compute the dihedral angle (in radians) between the four particles
/** \ingroup helper
    See XYZ
 */
IMPCOREEXPORT double get_dihedral(XYZ a, XYZ b, XYZ c, XYZ d);

//! Apply a transformation to the particle
/** See XYZ
    See algebra::Transformation3D
*/
IMPCOREEXPORT void transform(XYZ a, const algebra::Transformation3D &tr);

IMP_DECORATORS(XYZ, XYZs, kernel::ParticlesTemp);

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_XY_Z_H */
