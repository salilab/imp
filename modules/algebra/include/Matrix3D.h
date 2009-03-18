/**
 *  \file Matrix3D.h
 *  \brief Management of 3D matrices (volumes) of data
 *  \author Javier Velazquez-Muriel
 *  Copyright 2007-9 Sali Lab. All rights reserved.
*/

#ifndef IMPALGEBRA_MATRIX_3D_H
#define IMPALGEBRA_MATRIX_3D_H

#include "IMP/base_types.h"
#include "IMP/exception.h"
#include "MultiArray.h"


IMPALGEBRA_BEGIN_NAMESPACE

//! Template class for managing 3D matrices. This class is based on
//! boost multi_array
template<typename T>
class Matrix3D: public MultiArray<T,3>
{
public:
    typedef boost::multi_array_types::index index;
    typedef MultiArray<T,3> MA3;
    typedef Matrix3D<T> This;

  //! Empty constructor
  Matrix3D() : MA3() {
  }

  //! Constructor
  Matrix3D(int Zdim, int Ydim, int Xdim) : MA3() {
    resize(Zdim, Ydim, Xdim);
  }

  // Copy constructor
  Matrix3D(const This& v) : MA3() {
    this->reshape(v);
    this->copy(v);
  }

  //! Resizes the matrix
  /**
   * \param[in] Zdim Number of slices
   * \param[in] Ydim Number of rows
   * \param[in] Xdim Number of columns
   */
  void resize(int Zdim, int Ydim, int Xdim) {
    typename This::extent_gen extents;
    MA3::resize(extents[Zdim][Ydim][Xdim]);
  }

  //! Resizes the matrix copying the size of a given one
  /**
   * \param[in] v Matrix3D whose size to copy
   */
  template<typename T1>
  void resize(const Matrix3D<T1>& v) {
    this->resize(v.shape()[0], v.shape()[1], v.shape()[2]);
  }


  //! Reshapes the matrix copying the size and range of a given one
  /**
   * \param[in] v Matrix3D whose shape to copy
   */
  template<typename T1>
  void reshape(const Matrix3D<T1>& v) {
    boost::array<index,3> shape,start;
    int dims=v.num_dimensions();
    for(int i=0;i<dims;i++) {
      shape[i]=v.shape()[i];
      start[i]=v.index_bases()[i];
    }
    resize(shape[0], shape[1], shape[2]);
    MA3::reindex(start);
 }

  //! Copies the contents of a matrix to this one (does not resize this one).
  //! For guarantee a copy with resizing use operator=
  /**
   * \param[in] v Matrix3D whose contents to copy
   */
  void copy(const This& v) {
    std::vector<index> idx(3);
    while (internal::roll_inds(idx, v.shape(),v.index_bases())) {
      (*this)(idx) = v(idx);
    }
  }

  //! Access operator. The returned element is the LOGICAL element of the
  //! matrix, NOT the direct one
  /**
   * \param[in] k first index
   * \param[in] j second index
   * \param[in] i third index
   */
  T& operator()(int k,int j, int i) const {
    if (this->get_start(0) <= k && k <= this->get_finish(0) &&
        this->get_start(1) <= j && j <= this->get_finish(1) &&
        this->get_start(2) <= i && i <= this->get_finish(2)) {
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
  int get_slices() {
    return (int)this->get_size(0);
  }

  //! Returns the number of rows in the matrix
  int get_rows() const {
    return (int)this->get_size(1);
  }

  //! Returns the number of columns in the matrix
  int get_columns() const {
    return (int)this->get_size(2);
  }


  void operator=(const This& v) {
    this->reshape(v);
    this->copy(v);
  }

  //! Sum operator
  This operator+(const This& v) const {
    This result;
    result.resize(*this);
    internal::operate_arrays(*this, v, result, '+');
    return result;
  }

  //! Minus operator
  This operator-(const This& v) const {
    This result;
    result.resize(*this);
    internal::operate_arrays(*this, v, result, '-');
    return result;
  }

  //! Multiplication operator
  This operator*(const This& v) const {
    This result;
    result.resize(*this);
    internal::operate_arrays(*this, v, result, '*');
    return result;
  }

  //! Division operator
  This operator/(const This& v) const {
    This result;
    result.resize(*this);
    internal::operate_arrays(*this, v, result, '/');
    return result;
  }

  //! Sum operator for an array and a scalar
  This operator+(const T& v) const {
    This result;
    result.resize(*this);
    internal::operate_array_and_scalar(*this, v, result, '+');
    return result;
  }

  //! Minus operator for an array and a scalar
  This operator-(const T& v) const {
    This result;
    result.resize(*this);
    internal::operate_array_and_scalar(*this, v, result, '-');
    return result;
  }

  //! Multiplication operator for an array and a scalar
  This operator*(const T& v) const {
    This result;
    result.resize(*this);
    internal::operate_array_and_scalar(*this, v, result, '*');
    return result;
  }

  //! Division operator for an array and a scalar
  This operator/(const T& v) const {
    This result;
    result.resize(*this);
    internal::operate_array_and_scalar(*this, v, result, '/');
    return result;
  }

  //! Addition operator
  This& operator+=(const This& v) {
    internal::operate_arrays(*this, v, *this, '+');
    return *this;
  }

  //! Substraction operator
  This& operator-=(const This& v) {
    internal::operate_arrays(*this, v, *this, '-');
    return *this;
  }

  //! Multiplication operator
  This& operator*=(const This& v) {
    internal::operate_arrays(*this, v, *this, '*');
    return *this;
  }

  //! Division operator
  This& operator/=(const This& v) {
    internal::operate_arrays(*this, v, *this, '/');
    return *this;
  }

  //! Addition operator for an array and a scalar
  This& operator+=(const T& v) {
    internal::operate_array_and_scalar(*this, v, *this, '+');
    return *this;
  }

  //! Substraction operator for an array and a scalar
  This& operator-=(const T& v) {
    internal::operate_array_and_scalar(*this, v, *this, '-');
    return *this;
  }

  //! Multiplication operator for an array and a scalar
  This& operator*=(const T& v) {
    internal::operate_array_and_scalar(*this, v, *this, '*');
    return *this;
  }

  //! Division operator for an array and a scalar
  This& operator/=(const T& v) {
    internal::operate_array_and_scalar(*this, v, *this, '/');
    return *this;
  }

#ifndef SWIG
  //! Sum operator for a scalar and an array
  friend This operator+(const T& X, const This& a1) {
    This result;
    result.resize(a1);
    internal::operate_scalar_and_array(X, a1, result, '+');
    return result;
  }

  //! Minus operator for a scalar and an array
  friend This operator-(const T& X, const This& a1) {
    This result;
    result.resize(a1);
    internal::operate_scalar_and_array(X, a1, result, '-');
    return result;
  }

  //! Multiplication operator for a scalar and an array
  friend This operator*(const T& X, const This& a1) {
    This result;
    result.resize(a1);
    internal::operate_scalar_and_array(X, a1, result, '*');
    return result;
  }

  //! Division operator for a scalar and an array
  friend This operator/(const T& X, const This& a1) {
    This result;
    result.resize(a1);
    internal::operate_scalar_and_array(X, a1, result, '/');
    return result;
  }
#endif

protected:
};

IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_MATRIX_3D_H */
