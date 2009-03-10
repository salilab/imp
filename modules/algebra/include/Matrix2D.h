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
class Matrix2D: public IMP::algebra::MultiArray<T,2>
{
public:
    typedef boost::multi_array_types::index index;
    typedef IMP::algebra::MultiArray<T,2> MA2;
    typedef IMP::algebra::Matrix2D<T> This;

  //! Empty constructor
  Matrix2D() : MA2() {
  }

  //! Constructor
  /**
   * \param[in] Ydim Number of rows
   * \param[in] Xdim Number of columns
   */
  Matrix2D(int Ydim, int Xdim) : MA2() {
    resize(Ydim, Xdim);
  }

  // Copy constructor
  Matrix2D(const This& v) : MA2() {
    this->reshape(v);
    this->copy(v);
  }

  //! Resizes the matrix
  /**
   * \param[in] Ydim Number of rows
   * \param[in] Xdim Number of columns
   */
  void resize(int Ydim, int Xdim) {
    typename This::extent_gen extents;
    MA2::resize(extents[Ydim][Xdim]);
  }

  //! Resizes the matrix copying the size of a given one
  /**
   * \param[in] m2 Matrix2D whose size to copy
   */
  template<typename T1>
  void resize(const IMP::algebra::Matrix2D<T1>& m2) {
    this->resize(m2.shape()[0], m2.shape()[1]);
  }

  //! Reshapes the matrix copying the size and range of a given one
  /**
   * \param[in] v Matrix2D whose shape to copy
   */
  template<typename T1>
  void reshape(const IMP::algebra::Matrix2D<T1>& v) {
    boost::array<index,2> shape,start;
    int dims=v.num_dimensions();
    for(int i=0;i<dims;i++) {
      shape[i]=v.shape()[i];
      start[i]=v.index_bases()[i];
    }
    resize(shape[0], shape[1]);
    MA2::reindex(start);
 }

  //! Copies the contents of a matrix to this one (does not resize this one).
  //! For guarantee a copy with resizing use operator=
  /**
   * \param[in] v Matrix2D whose contents to copy
   */
  void copy(const This& v) {
    std::vector<index> idx(2);
    while (internal::roll_inds(idx, v.shape(),v.index_bases())) {
      (*this)(idx) = v(idx);
    }
  }

  //! Returns the number of rows in the matrix
  int get_rows() const {
    return (int)this->get_size(0);
  }

  //! Returns the number of columns in the matrix
  int get_columns() const {
    return (int)this->get_size(1);
  }

  //! Returns a matrix with this matrix transposed. The original one is not
  //! modified
  This transpose() {
    This aux(get_columns(), get_rows());
    for (int j = 0;j < get_rows();j++) {
      for (int i = 0;i < get_columns();i++) {
        aux(i, j) = (*this)(j, i);
      }
    }
    return aux;
  }

  //! Physicial access to the elements of the matrix
  /**
   * \param[in] j physical row to access
   * \param[in] i physical Column to access
   */
  T& physical_get(const int j,const int i) const {
    if (0<=j && j<get_rows() && 0<=i && i<get_columns()) {
      return (*this)(j+this->get_start(0),i+this->get_start(1));
    } else {
      String msg = "Matri2D::physical_get: index out of range." ;
      throw ValueException(msg.c_str());
    }
  }

  //! Physicial set of the elements of the matrix
  /**
   * \param[in] j physical row to access
   * \param[in] i physical Column to access
   */
  void physical_set(const int j,const int i,const T& val) {
    if (0<=j && j<get_rows() && 0<=i && i < get_columns()) {
      (*this)(j+this->get_start(0),i+this->get_start(1))=val;
    } else {
      String msg = "Matri2D::physical_set: index out of range." ;
      throw ValueException(msg.c_str());
    }
  }

  //! Access operator. The returned element is the LOGICAL element of the
  //! matrix, NOT the direct one
  /**
   * \param[in] j Row to access
   * \param[in] i Column to access
   */
  T& operator()(int j, int i) const {
    if (this->get_start(0) <= j && j <= this->get_finish(0) &&
        this->get_start(1) <= i && i <= this->get_finish(1)) {
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
  /**
   * \param[in] v The matrix to add
   */
  This operator+(const This& v)
  const {
    This result;
    result.resize(*this);
    internal::operate_arrays(*((MA2*)this),(MA2&)v,(MA2&)result, '+');
    return result;
  }

  //! Minus operator
  This operator-(const This& v) const {
    This result;
    result.resize(*this);
    internal::operate_arrays(*((MA2*)this),(MA2&)v,(MA2&)result, '-');
    return result;
  }

  //! Multiplication operator
  This operator*(const This& v) const {
    This result;
    result.resize(*this);
    internal::operate_arrays(*((MA2*)this),(MA2&)v,(MA2&)result, '*');
    return result;
  }

  //! Division operator
  This operator/(const This& v)
  const {
    This result;
    result.resize(*this);
    internal::operate_arrays(*((MA2*)this),(MA2&)v,(MA2&)result, '/');
    return result;
  }

  //! Sub operator for an array and a scalar
  This operator+(const T& v) const {
    This result;
    result.resize(*this);
    internal::operate_array_and_scalar(*((MA2*)this),v,(MA2&)result, '+');
    return result;
  }

  //! Minus operator for an array and a scalar
  This operator-(const T& v) const {
    This result;
    result.resize(*this);
    internal::operate_array_and_scalar(*((MA2*)this),v,(MA2&)result, '-');
    return result;
  }

  //! Multiplication operator for an array and a scalar
  This operator*(const T& v) const {
    This result;
    result.resize(*this);
    internal::operate_array_and_scalar(*((MA2*)this),v,(MA2&)result, '*');
    return result;
  }

  //! Division operator for an array and a scalar
  This operator/(const T& v) const {
    This result;
    result.resize(*this);
    internal::operate_array_and_scalar(*((MA2*)this),v,(MA2&)result, '/');
    return result;
  }


  //! Addition operator
  void operator+=(const This& v) const {
    internal::operate_arrays((MA2&)(*this),(MA2)v,(MA2&)(*this), '+');
  }

  //! Substraction operator
  void operator-=(const This& v) const {
    internal::operate_arrays(*((MA2*)this),(MA2)v,*((MA2*)this), '-');
  }

  //! Multiplication operator
  void operator*=(const This& v) const {
    internal::operate_arrays(*((MA2*)this),(MA2)v,*((MA2*)this), '*');
  }

  //! Division operator
  void operator/=(const This& v) const {
    internal::operate_arrays(*((MA2*)this),(MA2)v,*((MA2*)this), '/');
  }

  //! Addition operator for an array and a scalar
  void operator+=(const T& v) const {
    internal::operate_array_and_scalar(*((MA2*)this),
                                       v,
                                       *((MA2*)this), '+');
  }

  //! Substraction operator for an array and a scalar
  void operator-=(const T& v) const {
    internal::operate_array_and_scalar(*((MA2*)this),
                                       v,
                                       *((MA2*)this), '-');
  }

  //! Multiplication operator for an array and a scalar
  void operator*=(const T& v) const {
    internal::operate_array_and_scalar(*((MA2*)this),
                                       v,
                                       *((MA2*)this), '*');
  }

  //! Division operator for an array and a scalar
  void operator/=(const T& v) const {
    internal::operate_array_and_scalar(*((MA2*)this),
                                       v,
                                       *((MA2*)this), '/');
  }

  //! Sum operator for a scalar and an array
  friend This operator+(const T& X,
                         const This& a1) {
    This result;
    result.resize(*a1);
    internal::operate_scalar_and_array(X, (MA2&)a1, (MA2&)result, '+');
    return result;
  }

  //! Minus operator for a scalar and an array
  friend This operator-(const T& X,
                         const This& a1) {
    This result;
    result.resize(*a1);
    internal::operate_scalar_and_array(X, (MA2&)a1, (MA2&)result, '-');
    return result;
  }

  //! Multiplication operator for a scalar and an array
  friend This operator*(const T& X,const This& a1) {
    This result;
    result.resize(*a1);
    internal::operate_scalar_and_array(X, (MA2&)a1, (MA2&)result, '*');
    return result;
  }


  //! Division operator for a scalar and an array
  friend This operator/(const T& X,
        const This& a1) {
    This result;
    result.resize(*a1);
    internal::operate_scalar_and_array(X, (MA2&)a1, (MA2&)result, '/');
    return result;
  }

#endif
  //! Determinant (only for 2x2)
  double det() {
    return (double)(physical_get(0,0)*physical_get(1,1) -
                    physical_get(1,0)*physical_get(0,1));
  }

protected:
};

IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_MATRIX_2D_H */
