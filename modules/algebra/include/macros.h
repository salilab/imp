/**
 *  \file algebra/macros.h    \brief Various important macros
 *                           for implementing geometry.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_MACROS_H
#define IMPALGEBRA_MACROS_H

#ifdef IMP_DOXYGEN
//! implement the needed namespace methods for a geometry type
/** These are
    - IMP::algebra::get_surface_area()
    - IMP::algebra::get_volume()
    - IMP::algebra::get_bounding_box()
    - output to a stream

    The name of the argument is g and the code snipets should return
    the expected value.
*/
#define IMP_VOLUME_GEOMETRY_METHODS(Name, area, volume, bounding_box)

//! implement the needed namespace methods for a geometry type
/** These are
    - IMP::algebra::get_surface_area()
    - IMP::algebra::get_volume()
    - IMP::algebra::get_bounding_box()
    - output to a stream

    The name of the argument is g and the code snipets should return
    the expected value.
*/
#define IMP_VOLUME_GEOMETRY_METHODS_D(Name, area, volume, bounding_box)

//! implement the needed namespace methods for a geometry type
/** These are
    - IMP::algebra::get_bounding_box()
    - output to a stream

    The name should not include the D.

    The name of the argument is g and the code snipets should return
    the expected value.
*/
#define IMP_LINEAR_GEOMETRY_METHODS(Name, bounding_box)

//! implement the needed namespace methods for a geometry type
/** These are
    - IMP::algebra::get_surface_area()
    - IMP::algebra::get_bounding_box()
    - output to a stream

    The name of the argument is g and the code snipets should return
    the expected value.
*/
#define IMP_AREA_GEOMETRY_METHODS(Name, area, bounding_box)

#else
#define IMP_VOLUME_GEOMETRY_METHODS(Name, area, volume, bounding_box)  \
  inline double get_surface_area(const Name &g) {                      \
    area;                                                              \
  }                                                                    \
  inline double get_volume(const Name &g) {                            \
    volume;                                                            \
  }                                                                    \
  inline BoundingBox3D get_bounding_box(const Name &g) {               \
    bounding_box;                                                      \
  }                                                                    \
  IMP_OUTPUT_OPERATOR(Name)                                            \
  typedef std::vector<Name> Name##s

#define IMP_VOLUME_GEOMETRY_METHODS_D(Name, area, volume, bounding_box) \
  template <unsigned int D>                                             \
  double get_surface_area(const Name##D<D> &g) {                        \
    area;                                                               \
  }                                                                     \
  template <unsigned int D>                                             \
  double get_volume(const Name##D<D> &g) {                              \
    volume;                                                             \
  }                                                                     \
  template <unsigned int D>                                             \
  BoundingBoxD<D> get_bounding_box(const Name##D<D> &g) {               \
    bounding_box;                                                       \
  }                                                                     \
  IMP_OUTPUT_OPERATOR_D(Name##D)                                        \
  IMPALGEBRA_EXPORT_TEMPLATE(Name##D<3>);                               \
  IMP_NO_SWIG(typedef Name##D<3> Name##3D;)                             \
  typedef std::vector<Name##3D> Name##3Ds

#define IMP_LINEAR_GEOMETRY_METHODS(Name, bounding_box)                \
  inline BoundingBox3D get_bounding_box(const Name &g) {               \
    bounding_box;                                                      \
  }                                                                    \
  IMP_OUTPUT_OPERATOR(Name)                                            \
  typedef std::vector<Name> Name##s

#define IMP_AREA_GEOMETRY_METHODS(Name, area, bounding_box)            \
  inline double get_area(const Name &g) {                              \
    area;                                                              \
  }                                                                    \
  inline BoundingBox3D get_bounding_box(const Name &g) {               \
    bounding_box;                                                      \
  }                                                                    \
  IMP_OUTPUT_OPERATOR(Name)                                            \
  typedef std::vector<Name> Name##s

#endif

#endif  /* IMPALGEBRA_MACROS_H */
