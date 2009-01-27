/**
 *  \file Matrix2D.h
 *  \brief Management of 2D matrices.
 *  \author Javier Velazquez-Muriel
 *  Copyright 2007-8 Sali Lab. All rights reserved.
*/

#ifndef IMPALGEBRA_MATRIX_2D_H
#define IMPALGEBRA_MATRIX_2D_H

#include "IMP/base_types.h"
#include "IMP/exception.h"
#include "MultiArray.h"

IMPALGEBRA_BEGIN_NAMESPACE

//! Template class for managing 2D matrices. This class is based on
//! boost multi_array
template<typename T>
class IMPALGEBRAEXPORT Matrix2D: public MultiArray<T, 2>
{
public:
  //! Empty constructor
  Matrix2D() : MultiArray<T, 2>() {
  }

  //! Constructor
  /**
   * \param[in] Ydim Number of rows
   * \param[in] Xdim Number of columns
   */
  Matrix2D(Int Ydim, Int Xdim) : MultiArray<T, 2>() {
    resize(Ydim, Xdim);
  }


  //! Resizes the matrix
  /**
   * \param[in] Ydim Number of rows
   * \param[in] Xdim Number of columns
   */
  void resize(Int Ydim, Int Xdim) {
    std::vector<Int> shape(2);
    shape[0] = Ydim;
    shape[1] = Xdim;
    MultiArray<T, 2>::resize(shape);
  }

  //! Resizes the matrix copying the size of a given one
  /**
   * \param[in] m2 Matrix2D whose size to copy
   */
  template<typename T1>
  void resize(Matrix2D<T1>& m2) {
    this->resize(m2.shape()[0], m2.shape()[1]);
  }

  //! Get the origin of a given dimension
  Int get_start(const Int dim) const {
    return MultiArray<T, 2>::get_start(dim);
  }

  //! Set the origin of a given dimension
  void set_start(const Int dim, const Int value) {
    MultiArray<T, 2>::set_start(dim, value);
  }

  //! Set the origin of all the dimensions
  /**
   * \param[in] v Any class able to be accessed with []
   */
  template<typename T1>
  void set_start(const T1& v) {
    MultiArray<T, 2>::reindex(v);
  }

  //! Get the final index value for a given dimension
  Int get_finish(const Int dim) const {
    return MultiArray<T, 2>::get_finish(dim);
  }

  //! Returns the number of rows in the matrix
  Int get_rows() const {
    return (Int)this->get_size(0);
  }

  //! Returns the number of columns in the matrix
  Int get_columns() const {
    return (Int)this->get_size(1);
  }

  //! Returns a matrix with this matrix transposed. The original one is not
  //! modified
  void transpose() {
    Matrix2D<T> aux(get_columns(), get_rows());
    for (Int j = 0;j < get_rows();j++) {
      for (Int i = 0;i < get_columns();i++) {
        aux(i, j) = (*this)(j, i);
      }
    }
    return aux;
  }

  //! Access operator. The returned element is the LOGICAL element of the
  //! matrix, NOT the direct one
  /**
   * \param[in] j Row to access
   * \param[in] i Column to access
   */
  T& operator()(Int j, Int i) const {
    if (get_start(0) <= j && j <= get_finish(0) &&
        get_start(1) <= i && i <= get_finish(1)) {
      return (T&)((*this)[j][i]);
    } else {
      String msg = "Matri2D::(): Index out of range." ;
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
    return (T&)((*this)[idx[0]][idx[1]]);
  }

  //! Sum operator
  /**
   * \param[in] v The matrix to add
   */
  Matrix2D<T> operator+(const Matrix2D<T>& v) const {
    Matrix2D<T> result;
    result.copy_shape(*this);
    operate_arrays(*((MultiArray<T, 2>*)this),
                   (MultiArray<T, 2>&)v,
                   (MultiArray<T, 2>&)result, '+');
    return result;
  }

  //! Minus operator
  Matrix2D<T> operator-(const Matrix2D<T>& v) const {
    Matrix2D<T> result;
    result.copy_shape(*this);
    operate_arrays(*((MultiArray<T, 2>*)this),
                   (MultiArray<T, 2>&)v,
                   (MultiArray<T, 2>&)result, '-');
    return result;
  }

  //! Multiplication operator
  Matrix2D<T> operator*(const Matrix2D<T>& v) const {
    Matrix2D<T> result;
    result.copy_shape(*this);
    operate_arrays(*((MultiArray<T, 2>*)this),
                   (MultiArray<T, 2>&)v,
                   (MultiArray<T, 2>&)result, '*');
    return result;
  }

  //! Division operator
  Matrix2D<T> operator/(const Matrix2D<T>& v) const {
    Matrix2D<T> result;
    result.copy_shape(*this);
    operate_arrays(*((MultiArray<T, 2>*)this),
                   (MultiArray<T, 2>&)v,
                   (MultiArray<T, 2>&)result, '/');
    return result;
  }


  //! Addition operator
  void operator+=(const Matrix2D<T>& v) const {
    operate_arrays((MultiArray<T, 2>&)(*this),
                   (MultiArray<T, 2>)v,
                   (MultiArray<T, 2>&)(*this), '+');
  }

  //! Substraction operator
  void operator-=(const Matrix2D<T>& v) const {
    operate_arrays(*((MultiArray<T, 2>*)this),
                   (MultiArray<T, 2>)v,
                   *((MultiArray<T, 2>*)this), '-');
  }

  //! Multiplication operator
  void operator*=(const Matrix2D<T>& v) const {
    operate_arrays(*((MultiArray<T, 2>*)this),
                   (MultiArray<T, 2>)v,
                   *((MultiArray<T, 2>*)this), '*');
  }

  //! Division operator
  void operator/=(const Matrix2D<T>& v) const {
    operate_arrays(*((MultiArray<T, 2>*)this),
                   (MultiArray<T, 2>)v,
                   *((MultiArray<T, 2>*)this), '/');
  }

  //! Sum operator for an array and a scalar
  Matrix2D<T> operator+(const T& v) const {
    Matrix2D<T> result;
    result.copy_shape(*this);
    operate_array_and_scalar(*this, v, result, '+');
    return result;
  }

  //! Minus operator for an array and a scalar
  Matrix2D<T> operator-(const T& v) const {
    Matrix2D<T> result;
    result.copy_shape(*this);
    operate_array_and_scalar(*this, v, result, '-');
    return result;
  }

  //! Multiplication operator for an array and a scalar
  Matrix2D<T> operator*(const T& v) const {
    Matrix2D<T> result;
    result.copy_shape(*this);
    operate_array_and_scalar(*this, v, result, '*');
    return result;
  }


  //! Division operator for an array and a scalar
  Matrix2D<T> operator/(const T& v) const {
    Matrix2D<T> result;
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
    operate_array_and_scalar(*((MultiArray<T, 2>*)this),
                             v,
                             *((MultiArray<T, 2>*)this), '/');
  }

  //! Sum operator for a scalar and an array
  friend Matrix2D<T> operator+(const T& X, const Matrix2D<T>& a1) {
    Matrix2D<T> result;
    result.copy_shape(*a1);
    operate_scalar_and_array(X, a1, result, '+');
    return result;
  }

  //! Minus operator for a scalar and an array
  friend Matrix2D<T> operator-(const T& X, const Matrix2D<T>& a1) {
    Matrix2D<T> result;
    result.copy_shape(*a1);
    operate_scalar_and_array(X, a1, result, '-');
    return result;
  }

  //! Multiplication operator for a scalar and an array
  friend Matrix2D<T> operator*(const T& X, const Matrix2D<T>& a1) {
    Matrix2D<T> result;
    result.copy_shape(*a1);
    operate_scalar_and_array(X, a1, result, '*');
    return result;
  }

  //! Division operator for a scalar and an array
  friend Matrix2D<T> operator/(const T& X, const Matrix2D<T>& a1) {
    Matrix2D<T> result;
    result.copy_shape(*a1);
    operate_scalar_and_array(X, a1, result, '/');
    return result;
  }

  //! Determinant (only for 2x2)
  Float det() {
    return (Float)((*this)(0, 0)*(*this)(1, 1) - (*this)(1, 0)*(*this)(0, 1));
  }

protected:
};


IMPALGEBRA_END_NAMESPACE


#endif  /* IMPALGEBRA_MATRIX_2D_H */
