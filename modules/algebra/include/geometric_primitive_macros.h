/**
 *  \file IMP/algebra/geometric_primitive_macros.h
 *  \brief Various important macros
 *                           for implementing geometry.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_GEOMETRIC_PRIMITIVE_MACROS_H
#define IMPALGEBRA_GEOMETRIC_PRIMITIVE_MACROS_H


//! implement the needed namespace methods for a geometry type
/** These are
    - IMP::algebra::get_surface_area()
    - IMP::algebra::get_volume()
    - IMP::algebra::get_bounding_box()
    - output to a stream

    The name of the argument is g and the code snipets should return
    the expected value.
*/
#define IMP_VOLUME_GEOMETRY_METHODS(Name, name, area, volume, bounding_box) \
  IMP_VALUES(Name, Name##s);                                            \
  /** \relates Name */                                                  \
  inline double get_surface_area(const Name &g) {                      \
    area;                                                              \
  }                                                                    \
  /** \relates Name */                                                  \
  inline double get_volume(const Name &g) {                            \
    volume;                                                            \
  }                                                                    \
  /** \relates Name */                                                  \
  inline BoundingBoxD<3> get_bounding_box(const Name &g) {             \
    bounding_box;                                                      \
  }                                                                    \
  /** \relates Name */                                                  \
  inline const Name &get_##name##_geometry(const Name &g) {return g;}  \
  /** \relates Name */                                                  \
  IMP_NO_SWIG(inline void set_##name##_geometry(Name &g, const Name &v) {g=v;})

//! implement the needed namespace methods for a geometry type
/** These are
    - IMP::algebra::get_surface_area()
    - IMP::algebra::get_volume()
    - IMP::algebra::get_bounding_box()
    - output to a stream

    The name of the argument is g and the code snipets should return
    the expected value.
*/
#define IMP_VOLUME_GEOMETRY_METHODS_D(Name, name, area, volume, bounding_box) \
  /** \relates Name */                                                  \
  inline double get_surface_area(const Name##D<3> &g) {                 \
    area;                                                               \
  }                                                                     \
  /** \relates Name */                                                  \
  inline double get_volume(const Name##D<3> &g) {                       \
    volume;                                                             \
  }                                                                     \
  /** \relates Name */                                                  \
  template <int D>                                                      \
  inline BoundingBoxD<D> get_bounding_box(const Name##D<D> &g) {        \
    bounding_box;                                                       \
  }                                                                     \
  /** \relates Name */                                                  \
  IMP_NO_SWIG(template <int D>                                          \
  inline void set_##name##_d_geometry(Name##D<D> &g,                    \
                                      const Name##D<D> &v) {            \
    g=v;                                                                \
  }                                                                     \
  /** \relates Name */                                                  \
  template <int D>                                             \
 inline const Name##D<D> &get_##name##_d_geometry(const Name##D<D> &g) { \
    return g;                                                           \
  })                                                                    \
  /** Typedef for python. */                                            \
  typedef Name##D<1> Name##1D;                                          \
  IMP_VALUES(Name##1D, Name##1Ds);                                      \
  /** Typedef for python. */                                            \
  typedef Name##D<2> Name##2D;                                          \
  IMP_VALUES(Name##2D, Name##2Ds);                                      \
  /** Typedef for python.  */                                           \
  typedef Name##D<3> Name##3D;                                          \
  IMP_VALUES(Name##3D, Name##3Ds);                                      \
  /** Typedef for python. */                                            \
  typedef Name##D<4> Name##4D;                                          \
  IMP_VALUES(Name##4D, Name##4Ds);                                      \
  /** Typedef for python. */                                            \
  typedef Name##D<5> Name##5D;                                          \
  IMP_VALUES(Name##5D, Name##5Ds);                                      \
  /** Typedef for python. */                                            \
  typedef Name##D<6> Name##6D;                                          \
  IMP_VALUES(Name##6D, Name##6Ds);                                      \
  /** Typedef for python. */                                            \
  typedef Name##D<-1> Name##KD;                                         \
  IMP_VALUES(Name##KD, Name##KDs)


//! implement the needed namespace methods for a geometry type
/** These are
    - IMP::algebra::get_bounding_box()
    - output to a stream

    The name should not include the D.

    The name of the argument is g and the code snipets should return
    the expected value.
*/
#define IMP_LINEAR_GEOMETRY_METHODS(Name, name, bounding_box)          \
  IMP_VALUES(Name, Name##s);                                           \
  /** \relates Name */                                                  \
  inline BoundingBoxD<3> get_bounding_box(const Name &g) {             \
    bounding_box;                                                      \
  }                                                                    \
  /** \relates Name */                                                  \
  inline const Name &get_##name##_geometry(const Name &g) {return g;}  \
  IMP_NO_SWIG(inline void set_##name##_geometry( Name &g, const Name &gi)\
              {g=gi;})                                                  \

//! implement the needed namespace methods for a geometry type
/** These are
    - IMP::algebra::get_surface_area()
    - IMP::algebra::get_bounding_box()
    - output to a stream

    The name of the argument is g and the code snipets should return
    the expected value.
*/
#define IMP_AREA_GEOMETRY_METHODS(Name, name, area, bounding_box)      \
  IMP_VALUES(Name, Name##s);                                           \
  /** \relates Name */                                                  \
  inline double get_area(const Name &g) {                              \
    area;                                                              \
  }                                                                    \
  /** \relates Name */                                                  \
  inline BoundingBoxD<3> get_bounding_box(const Name &g) {             \
    bounding_box;                                                      \
  }                                                                    \
  /** \relates Name */                                                  \
  inline const Name &get_##name##_geometry(const Name &g) {return g;}  \
  /** \relates Name */                                                  \
  IMP_NO_SWIG(inline void set_##name##_geometry(Name &g, const Name &v) {g=v;})\


//! implement the needed namespace methods for a geometry type
/** These are
    - IMP::algebra::get_surface_area()
    - IMP::algebra::get_bounding_box()
    - output to a stream

    The name of the argument is g and the code snipets should return
    the expected value.
*/
#define IMP_AREA_GEOMETRY_METHODS_D(Name, name, area, bounding_box)     \
  /** \relates Name */                                                  \
  template <int D>                                                      \
  inline double get_area(const Name##D<D> &g) {                         \
    area;                                                               \
  }                                                                     \
  /** \relates Name */                                                  \
  template <int D>                                                      \
  inline BoundingBoxD<D> get_bounding_box(const Name##D<D> &g) {        \
    bounding_box;                                                       \
  }                                                                     \
  /** \relates Name */                                                  \
  template <int D>                                                      \
  inline const Name &get_##name##_d_geometry(const Name##D<D> &g) {     \
    return g;}                                                          \
  /** \relates Name */                                                  \
  IMP_NO_SWIG(template <int D>                                          \
  inline void set_##name##_d_geometry(Name &g, const Name##D<D> &v) {   \
                g=v;})                                                  \
  /** Typedef for python. */                                            \
  typedef Name##D<1> Name##1D;                                          \
  IMP_VALUES(Name##1D, Name##1Ds);                                      \
  /** Typedef for python. */                                            \
  typedef Name##D<2> Name##2D;                                          \
  IMP_VALUES(Name##2D, Name##2Ds);                                      \
  /** Typedef for python. */                                            \
  typedef Name##D<3> Name##3D;                                          \
  IMP_VALUES(Name##3D, Name##3Ds);                                      \
  /** Typedef for python. */                                            \
  typedef Name##D<4> Name##4D;                                          \
  IMP_VALUES(Name##4D, Name##4Ds);                                      \
  /** Typedef for python. */                                            \
  typedef Name##D<5> Name##5D;                                          \
  IMP_VALUES(Name##5D, Name##5Ds);                                      \
  /** Typedef for python.*/                                             \
  typedef Name##D<6> Name##6D;                                          \
  IMP_VALUES(Name##6D, Name##6Ds);                                      \
  /** Typedef for python. */                                            \
  typedef Name##D<-1> Name##KD;                                         \
  IMP_VALUES(Name##KD, Name##KDs)


#endif  /* IMPALGEBRA_GEOMETRIC_PRIMITIVE_MACROS_H */
