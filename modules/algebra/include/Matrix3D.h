/**
 *  \file Matrix3D.h
 *  \brief Management of 3D matrices (volumes) of data
 *  \author Javier Velazquez-Muriel
 *  Copyright 2007-8 Sali Lab. All rights reserved.
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
class IMPALGEBRAEXPORT Matrix3D: public IMP::algebra::MultiArray<T,3>
{
public:
    typedef boost::multi_array_types::index index;
    typedef IMP::algebra::MultiArray<T,3> MA3;
    typedef IMP::algebra::Matrix3D<T> This;

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
  void resize(const IMP::algebra::Matrix3D<T1>& v) {
    this->resize(v.shape()[0], v.shape()[1], v.shape()[2]);
  }


  //! Reshapes the matrix copying the size and range of a given one
  /**
   * \param[in] v Matrix3D whose shape to copy
   */
  template<typename T1>
  void reshape(const IMP::algebra::Matrix3D<T1>& v) {
    boost::array<index,3> shape,start;
    int dims=v.num_dimensions();
    for(int i=0;i<dims;i++) {
      shape[i]=v.shape()[i];
      start[i]=v.index_bases()[i];
    }
    MA3::resize(shape);
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

  //! Get the origin of a given dimension
  int get_start(const int dim) const {
    return MA3::get_start(dim);
  }

  //! Set the origin of a given dimension
  void set_start(const int dim, const int value) {
    MA3::set_start(dim, value);
  }

  //! Set the origin of all the dimensions
  /**
   * \param[in] v Any class able to be accessed with []
   */
  template<typename T1>
  void set_start(const T1& v) {
    MA3::reindex(v);
  }

  //! Get the final index value for a given dimension
  int get_finish(const int dim) const {
    return MA3::get_finish(dim);
  }


  //! Access operator. The returned element is the LOGICAL element of the
  //! matrix, NOT the direct one
  /**
   * \param[in] k first index
   * \param[in] j second index
   * \param[in] i third index
   */
  T& operator()(int k,int j, int i) const {
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

  // Functions to help wrapping operators
  This test_add(const This& v) const {return (*this) + v;}
  This test_sub(const This& v) const {return (*this) - v;}
  This test_mul(const This& v) const {return (*this) * v;}
  This test_div(const This& v) const {return (*this) / v;}
  This test_add_f(const T& v) const {return (*this) + v;}
  This test_sub_f(const T& v) const {return (*this) - v;}
  This test_mul_f(const T& v) const {return (*this) * v;}
  This test_div_f(const T& v) const {return (*this) / v;}


#ifndef SWIG
  //! Sum operator
  This operator+(const This& v) const {
    This result;
    result.resize(*this);
    internal::operate_arrays(*((MA3*)this),(MA3&)v,(MA3&)result, '+');
    return result;
  }

  //! Minus operator
  This operator-(const This& v) const {
    This result;
    result.resize(*this);
    internal::operate_arrays(*((MA3*)this),(MA3&)v,(MA3&)result, '-');
    return result;
  }

  //! Multiplication operator
  This operator*(const This& v) const {
    This result;
    result.resize(*this);
    internal::operate_arrays(*((MA3*)this),(MA3&)v,(MA3&)result, '*');
    return result;
  }

  //! Division operator
  This operator/(const This& v) const {
    This result;
    result.resize(*this);
    internal::operate_arrays(*((MA3*)this),(MA3&)v,(MA3&)result, '/');
    return result;
  }

  //! Sum operator for an array and a scalar
  This operator+(const T& v) const {
    This result;
    result.resize(*this);
    internal::operate_array_and_scalar(*((MA3*)this), v, (MA3&)result, '+');
    return result;
  }

  //! Minus operator for an array and a scalar
  This operator-(const T& v) const {
    This result;
    result.resize(*this);
    internal::operate_array_and_scalar(*((MA3*)this), v, (MA3&)result, '-');
    return result;
  }

  //! Multiplication operator for an array and a scalar
  This operator*(const T& v) const {
    This result;
    result.resize(*this);
    internal::operate_array_and_scalar(*((MA3*)this),v,(MA3&)result, '*');
    return result;
  }

  //! Division operator for an array and a scalar
  This operator/(const T& v) const {
    This result;
    result.resize(*this);
    internal::operate_array_and_scalar(*((MA3*)this),v,(MA3&)result, '/');
    return result;
  }

  //! Addition operator
  void operator+=(const This& v) const {
    internal::operate_arrays(*((MA3*)this),(MA3&)v,*((MA3*)this), '+');
  }

  //! Substraction operator
  void operator-=(const This& v) const {
    internal::operate_arrays(*((MA3*)this),(MA3&)v,*((MA3*)this), '-');
  }

  //! Multiplication operator
  void operator*=(const This& v) const {
    internal::operate_arrays(*((MA3*)this),(MA3&)v,*((MA3*)this), '*');
  }

  //! Division operator
  void operator/=(const This& v) const {
    internal::operate_arrays(*((MA3*)this),(MA3&)v,*((MA3*)this), '/');
  }

  //! Addition operator for an array and a scalar
  void operator+=(const T& v) const {
    internal::operate_array_and_scalar(*((MA3*)this), v, *((MA3*)this), '+');
  }

  //! Substraction operator for an array and a scalar
  void operator-=(const T& v) const {
    internal::operate_array_and_scalar(*((MA3*)this), v, *((MA3*)this), '-');
  }

  //! Multiplication operator for an array and a scalar
  void operator*=(const T& v) const {
    internal::operate_array_and_scalar(*((MA3*)this), v, *((MA3*)this), '*');
  }

  //! Division operator for an array and a scalar
  void operator/=(const T& v) const {
    internal::operate_array_and_scalar(*((MA3*)this), v, *((MA3*)this), '/');
  }

  //! Sum operator for a scalar and an array
  friend This operator+(const T& X, const This& a1) {
    This result;
    result.resize(a1);
    internal::operate_scalar_and_array(X, (MA3&)a1, (MA3&)result, '+');
    return result;
  }

  //! Minus operator for a scalar and an array
  friend This operator-(const T& X, const This& a1) {
    This result;
    result.resize(a1);
    internal::operate_scalar_and_array(X, (MA3&)a1, (MA3&)result, '-');
    return result;
  }

  //! Multiplication operator for a scalar and an array
  friend This operator*(const T& X, const This& a1) {
    This result;
    result.resize(a1);
    internal::operate_scalar_and_array(X, (MA3&)a1, (MA3&)result, '*');
    return result;
  }

  //! Division operator for a scalar and an array
  friend This operator/(const T& X, const This& a1) {
    This result;
    result.resize(a1);
    internal::operate_scalar_and_array(X, (MA3&)a1, (MA3&)result, '/');
    return result;
  }
#endif

protected:
};

IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_MATRIX_3D_H */
