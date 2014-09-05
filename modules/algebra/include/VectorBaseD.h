/**
 *  \file IMP/algebra/VectorBaseD.h   \brief Simple D vector class.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_VECTOR_BASE_D_H
#define IMPALGEBRA_VECTOR_BASE_D_H

#include <IMP/algebra/algebra_config.h>
#include "GeometricPrimitiveD.h"
#include <IMP/base/check_macros.h>
#include <IMP/base/exception.h>
#include <IMP/base/random.h>
#include <IMP/base/utility.h>
#include <IMP/base/types.h>
#include <boost/random/variate_generator.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/range.hpp>
#include "internal/vector.h"

#include <limits>
#include <cmath>
#include <boost/random/normal_distribution.hpp>
#include <boost/static_assert.hpp>

#if IMP_HAS_CHECKS >= IMP_USAGE
#define IMP_ALGEBRA_VECTOR_CHECK check_vector()
#define IMP_ALGEBRA_VECTOR_CHECK_INDEX(i) check_index(i)
#define IMP_ALGEBRA_VECTOR_CHECK_COMPATIBLE(o) \
  check_compatible_vector(o);                  \
  o.check_vector()
#else
#define IMP_ALGEBRA_VECTOR_CHECK
#define IMP_ALGEBRA_VECTOR_CHECK_INDEX(i)
#define IMP_ALGEBRA_VECTOR_CHECK_COMPATIBLE(o)
#endif

IMPALGEBRA_BEGIN_NAMESPACE
//! A Cartesian vector in D-dimensions.
/** Store a vector of Cartesian coordinates. It supports all expected
    mathematical operators, including using * for the dot product.
    \see Vector3D
    \see Vector2D

    \geometry
 */
template <int D>
class VectorBaseD : public GeometricPrimitiveD<D> {
  void check_vector() const {
    IMP_USAGE_CHECK(!data_.get_is_null(),
                    "Attempt to use uninitialized vector.");
  }
  template <int OD>
  void check_compatible_vector(const VectorBaseD<OD> &o) const {
    IMP_USAGE_CHECK_VARIABLE(o);
    IMP_USAGE_CHECK(o.get_dimension() == get_dimension(),
                    "Dimensions don't match: " << get_dimension() << " vs "
                                               << o.get_dimension());
  }
  void check_index(unsigned int i) const {
#if IMP_HAS_CHECKS < IMP_INTERNAL
    IMP_UNUSED(i);
#endif
    IMP_INTERNAL_CHECK(i < data_.get_dimension(),
                       "Invalid component of vector requested: "
                           << i << " of " << get_dimension());
  }

 public:
  /** Will accept a list of floats from Python. */
  template <class Range>
  explicit VectorBaseD(const Range &r) {
    if (D != -1 && static_cast<int>(boost::distance(r)) != D) {
      IMP_THROW("Expected " << D << " but got " << boost::distance(r),
                base::ValueException);
    }
    IMP_IF_CHECK(USAGE) {
      IMP_FOREACH(double f, r) {
        IMP_UNUSED(f);
        IMP_USAGE_CHECK(!base::is_nan(f), "NaN passed to constructor");
      }
    }
    data_.set_coordinates(boost::begin(r), boost::end(r));
  }

#ifndef SWIG
  template <class R>
  VectorBaseD<D> &operator=(const R &r) {
    if (D != -1 && static_cast<int>(boost::distance(r)) != D) {
      IMP_THROW("Expected " << D << " but got " << boost::distance(r),
                base::ValueException);
    }
    IMP_IF_CHECK(USAGE) {
      IMP_FOREACH(double f, r) {
        IMP_USAGE_CHECK(!base::is_nan(f), "NaN passed in equals");
      }
    }
    data_.set_coordinates(boost::begin(r), boost::end(r));
  }
  //! Return the ith Cartesian coordinate.
  /** In 3D use [0] to get the x coordinate etc. */
  inline double operator[](unsigned int i) const {
    IMP_ALGEBRA_VECTOR_CHECK_INDEX(i);
    IMP_ALGEBRA_VECTOR_CHECK;
    return data_.get_data()[i];
  }
  //! Return the ith Cartesian coordinate.
  /** In 3D use [0] to get the x coordinate etc. */
  inline double &operator[](unsigned int i) {
    IMP_ALGEBRA_VECTOR_CHECK_INDEX(i);
    return data_.get_data()[i];
  }

#endif
  //! Default constructor
  VectorBaseD() {}

  double get_scalar_product(const VectorBaseD<D> &o) const {
    IMP_ALGEBRA_VECTOR_CHECK_COMPATIBLE(o);
    IMP_ALGEBRA_VECTOR_CHECK;
    double ret = 0;
    for (unsigned int i = 0; i < get_dimension(); ++i) {
      ret += operator[](i) * o.operator[](i);
    }
    return ret;
  }

  double get_squared_magnitude() const { return get_scalar_product(*this); }

  double get_magnitude() const { return std::sqrt(get_squared_magnitude()); }

#ifndef IMP_DOXYGEN
  double operator*(const VectorBaseD<D> &o) const {
    return get_scalar_product(o);
  }

  VectorBaseD &operator+=(const VectorBaseD &o) {
    IMP_ALGEBRA_VECTOR_CHECK_COMPATIBLE(o);
    IMP_ALGEBRA_VECTOR_CHECK;
    for (unsigned int i = 0; i < get_dimension(); ++i) {
      operator[](i) += o[i];
    }
    return *this;
  }

  VectorBaseD &operator-=(const VectorBaseD &o) {
    IMP_ALGEBRA_VECTOR_CHECK_COMPATIBLE(o);
    IMP_ALGEBRA_VECTOR_CHECK;
    for (unsigned int i = 0; i < get_dimension(); ++i) {
      operator[](i) -= o[i];
    }
    return *this;
  }

  VectorBaseD &operator/=(double f) {
    IMP_ALGEBRA_VECTOR_CHECK;
    for (unsigned int i = 0; i < get_dimension(); ++i) {
      operator[](i) /= f;
    }
    return *this;
  }

  VectorBaseD &operator*=(double f) {
    IMP_ALGEBRA_VECTOR_CHECK;
    for (unsigned int i = 0; i < get_dimension(); ++i) {
      operator[](i) *= f;
    }
    return *this;
  }

  void show(std::ostream &out, std::string delim, bool parens = true) const {
    IMP_ALGEBRA_VECTOR_CHECK;
    if (parens) out << "(";
    for (unsigned int i = 0; i < get_dimension(); ++i) {
      out << operator[](i);
      if (i != get_dimension() - 1) {
        out << delim;
      }
    }
    if (parens) out << ")";
  }
  IMP_SHOWABLE_INLINE(VectorBaseD, show(out, ", "););
#endif

#ifndef SWIG
  typedef double *iterator;
  typedef const double *const_iterator;
  iterator begin() { return data_.get_data(); }
  iterator end() { return data_.get_data() + get_dimension(); }
  const_iterator begin() const { return data_.get_data(); }
  const_iterator end() const { return data_.get_data() + get_dimension(); }

  typedef double value_type;
  typedef std::random_access_iterator_tag iterator_category;
  typedef std::ptrdiff_t difference_type;
  typedef double *pointer;
  typedef double &reference;
  typedef const double &const_reference;

  static const int DIMENSION = D;
#endif

#ifndef SWIG
  // For some reason, this method breaks IMP::atom::get_rmsd() in Python, so
  // hide it from SWIG
  Floats get_coordinates() const {
    return Floats(begin(), end());
  }

  //! Return a pointer to the data stored.
  /** Useful for conversion to other types. */
  const double *get_data() const { return data_.get_data(); }
#endif
  unsigned int get_dimension() const { return data_.get_dimension(); }

 private:
  internal::VectorData<double, D, false> data_;
};

//! Returns a unit vector pointing at the same direction as this vector.
/**
   @note If the magnitude of this vector is smaller than 1e-12
         (an arbitrarily selected small number), returns a unit
         vector pointing at a random direction.
 */
template <class VT>
inline VT get_unit_vector(VT vt) {
  const double tiny_double = 1e-12;
  double mag = vt.get_magnitude();
  if (mag > tiny_double) {
    return vt / mag;
  } else {
    // avoid division by zero - return random unit v
    // NOTE: (1) avoids vector_generators / SphereD to prevent recursiveness
    //       (2) D might be -1, so use get_dimension()
    boost::variate_generator<base::RandomNumberGenerator,
                             boost::normal_distribution<> >
        generator(IMP::base::random_number_generator,
                  ::boost::normal_distribution<>(0, 1.0));
    for (unsigned int i = 0; i < vt.get_dimension(); ++i) {
      vt[i] = generator();
    }
    return get_unit_vector(vt);
  }
}

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_VECTOR_BASE_D_H */
