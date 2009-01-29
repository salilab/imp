/**
 *  \file Matrix3D.h
 *  \brief Management of 3D matrices (volumes) of data
 *  \author Javier Velazquez-Muriel
 *  Copyright 2007-8 Sali Lab. All rights reserved.
*/

#ifndef IMPALGEBRA_MATRIX_3D_H
#define IMPALGEBRA_MATRIX_3D_H

#include "IMP/base_types.h"
#include "MultiArray.h"


IMPALGEBRA_BEGIN_NAMESPACE

//! Template class for managing 3D matrices. This class is based on
//! boost multi_array
template<typename T>
class IMPALGEBRAEXPORT Matrix3D: public MultiArray<T, 3>
{
public:
  //! Empty constructor
  Matrix3D() : MultiArray<T, 3>() {
  }

  //! Constructor
  Matrix3D(Int Zdim, Int Ydim, Int Xdim) : MultiArray<T, 3>() {
    resize(Zdim, Ydim, Xdim);
  }

  //! Resizes the matrix
  /**
   * \param[in] Zdim Number of slices
   * \param[in] Ydim Number of rows
   * \param[in] Xdim Number of columns
   */
  void resize(Int Zdim, Int Ydim, Int Xdim) {
    typename Matrix3D<T>::extent_gen extents;
    MultiArray<T, 3>::resize(extents[Zdim][Ydim][Xdim]);
  }


  //! Get the origin of a given dimension
  Int get_start(const Int dim) const {
    return MultiArray<T, 3>::get_start(dim);
  }

  //! Set the origin of a given dimension
  void set_start(const Int dim, const Int value) {
    MultiArray<T, 3>::set_start(dim, value);
  }

  //! Set the origin of all the dimensions
  /**
   * \param[in] v Any class able to be accessed with []
   */
  template<typename T1>
  void set_start(const T1& v) {
    MultiArray<T, 3>::reindex(v);
  }

  //! Get the final index value for a given dimension
  Int get_finish(const Int dim) const {
    return MultiArray<T, 3>::get_finish(dim);
  }


  //! Access operator. The returned element is the LOGICAL element of the
  //! matrix, NOT the direct one
  /**
   * \param[in] k first index
   * \param[in] j second index
   * \param[in] i third index
   */
  T& operator()(Int k,Int j, Int i) const {
    if (get_start(0) <= k && k <= get_finish(0) &&
        get_start(1) <= j && j <= get_finish(1) &&
        get_start(2) <= i && i <= get_finish(2)) {
      return (T&)(*this)[k][j][i];
    } else {
      String msg = "Matri3D::(): Index out of range." ;
      throw ValueException(msg.c_str());
    }
  }

  //! Access operator. The returned element is the LOGICAL element of the
  //! matrix, NOT the direct one
  /**
   * \param[in] idx must be a class supporting access via []
   */
  template<typename T1>
  T& operator()(T1& idx) const {
    return (T&)((*this)[idx[0]][idx[1]][idx[2]]);
  }

  //! Returns the number of slices in the matrix
  void get_slices() {
    this->shape()[0];
  }

  //! Returns the number of rows in the matrix
  void get_rows() {
    this->shape()[1];
  }

  //! Returns the number of columns in the matrix
  void get_columns() {
    this->shape()[2];
  }


  //! Sum operator
  Matrix3D<T> operator+(const Matrix3D<T>& v) const {
    Matrix3D<T> result;
    result.copy_shape(*this);
    operate_arrays(*this, v, result, '+');
    return result;
  }

  //! Minus operator
  Matrix3D<T> operator-(const Matrix3D<T>& v) const {
    Matrix3D<T> result;
    result.copy_shape(*this);
    operate_arrays(*this, v, result, '-');
    return result;
  }

  //! Multiplication operator
  Matrix3D<T> operator*(const Matrix3D<T>& v) const {
    Matrix3D<T> result;
    result.copy_shape(*this);
    operate_arrays(*this, v, result, '*');
    return result;
  }

  //! Division operator
  Matrix3D<T> operator/(const Matrix3D<T>& v) const {
    Matrix3D<T> result;
    result.copy_shape(*this);
    operate_arrays(*this, v, result, '/');
    return result;
  }


  //! Addition operator
  void operator+=(const Matrix3D<T>& v) const {
    operate_arrays(*this, v, *this, '+');
  }

  //! Substraction operator
  void operator-=(const Matrix3D<T>& v) const {
    operate_arrays(*this, v, *this, '-');
  }

  //! Multiplication operator
  void operator*=(const Matrix3D<T>& v) const {
    operate_arrays(*this, v, *this, '*');
  }

  //! Division operator
  void operator/=(const Matrix3D<T>& v) const {
    operate_arrays(*this, v, *this, '/');
  }

  //! Sum operator for an array and a scalar
  Matrix3D<T> operator+(const T& v) const {
    Matrix3D<T> result;
    result.copy_shape(*this);
    operate_array_and_scalar(*this, v, result, '+');
    return result;
  }

  //! Minus operator for an array and a scalar
  Matrix3D<T> operator-(const T& v) const {
    Matrix3D<T> result;
    result.copy_shape(*this);
    operate_array_and_scalar(*this, v, result, '-');
    return result;
  }

  //! Multiplication operator for an array and a scalar
  Matrix3D<T> operator*(const T& v) const {
    Matrix3D<T> result;
    result.copy_shape(*this);
    operate_array_and_scalar(*this, v, result, '*');
    return result;
  }


  //! Division operator for an array and a scalar
  Matrix3D<T> operator/(const T& v) const {
    Matrix3D<T> result;
    result.copy_shape(*this);
    operate_array_and_scalar(*this, v, result, '/');
    return result;
  }

  //! Addition operator for an array and a scalar
  void operator+=(const T& v) const {
    operate_array_and_scalar(*this, v, *this, '+');
  }

  //! Substraction operator for an array and a scalar
  void operator-=(const T& v) const {
    operate_array_and_scalar(*this, v, *this, '-');
  }

  //! Multiplication operator for an array and a scalar
  void operator*=(const T& v) const {
    operate_array_and_scalar(*this, v, *this, '*');
  }

  //! Division operator for an array and a scalar
  void operator/=(const T& v) const {
    operate_array_and_scalar(*this, v, *this, '/');
  }

  //! Sum operator for a scalar and an array
  friend Matrix3D<T> operator+(const T& X, const Matrix3D<T>& a1) {
    Matrix3D<T> result;
    result.copy_shape(*a1);
    operate_scalar_and_array(X, a1, result, '+');
    return result;
  }

  //! Minus operator for a scalar and an array
  friend Matrix3D<T> operator-(const T& X, const Matrix3D<T>& a1) {
    Matrix3D<T> result;
    result.copy_shape(*a1);
    operate_scalar_and_array(X, a1, result, '-');
    return result;
  }

  //! Multiplication operator for a scalar and an array
  friend Matrix3D<T> operator*(const T& X, const Matrix3D<T>& a1) {
    Matrix3D<T> result;
    result.copy_shape(*a1);
    operate_scalar_and_array(X, a1, result, '*');
    return result;
  }

  //! Division operator for a scalar and an array
  friend Matrix3D<T> operator/(const T& X, const Matrix3D<T>& a1) {
    Matrix3D<T> result;
    result.copy_shape(*a1);
    operate_scalar_and_array(X, a1, result, '/');
    return result;
  }

protected:
};

IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_MATRIX_3D_H */
