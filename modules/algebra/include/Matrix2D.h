/**
 *  \file Matrix2D.h
 *  \brief Management of 2D matrices.
 *  \author Javier Velazquez-Muriel
 *  Copyright 2007-9 Sali Lab. All rights reserved.
*/

#ifndef IMPALGEBRA_MATRIX_2D_H
#define IMPALGEBRA_MATRIX_2D_H

// #define DEBUG

#include "IMP/base_types.h"
#include "IMP/exception.h"
#include "MultiArray.h"
#include "VectorD.h"
#include <complex>

IMPALGEBRA_BEGIN_NAMESPACE

//! Template class for managing 2D matrices. This class is based on
//! boost multi_array
/**
  Check MultiArray class for a list of added functionality
**/
template<typename T>
class Matrix2D: public MultiArray<T,2>
{
public:
    typedef boost::multi_array_types::index index;
    typedef MultiArray<T,2> MA2;
    typedef Matrix2D<T> This;

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

  //! Copy constructor
  Matrix2D(const This& v) : MA2() {
    this->reshape(v);
    MA2::copy((MA2 &)v);
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
  void resize(const Matrix2D<T1>& m2) {
    this->resize(m2.shape()[0], m2.shape()[1]);
  }

  //! Reshapes the matrix copying the size and range of a given one
  /**
   * \param[in] v Matrix2D whose shape to copy
   */
  template<typename T1>
  void reshape(const Matrix2D<T1>& v) {
    boost::array<index,2> shape,start;
    int dims=v.num_dimensions();
    for(int i=0;i<dims;i++) {
      shape[i]=v.shape()[i];
      start[i]=v.index_bases()[i];
    }
    resize(shape[0], shape[1]);
    MA2::reindex(start);
 }

  //! Returns the number of rows in the matrix
  int get_number_of_rows() const {
    return (int)this->get_size(0);
  }

  //! Returns the number of columns in the matrix
  int get_number_of_columns() const {
    return (int)this->get_size(1);
  }

  //! Returns a matrix with this matrix transposed. The original one is not
  //! modified
  This transpose() {
    This aux(get_number_of_columns(), get_number_of_rows());
    for (int j = 0;j < get_number_of_rows();j++) {
      for (int i = 0;i < get_number_of_columns();i++) {
        aux(i, j) = (*this)(j, i);
      }
    }
    return aux;
  }

  //! Set the matrix to be a zero matrix
  void set_zero() {
    this->init_zeros();
  }

  //! Set the matrix to be an indentity matrix.
  void set_identity() {
    IMP_assert(is_square(), "the matrix must be square");
    this->init_zeros();
    for (int i = 0; i < get_number_of_rows(); i++) {
      (*this)(i,i)=1.;
    }
  }

  void show(std::ostream &o=std::cout) const {
    for(int i=0;i<get_number_of_rows();i++) {
      for(int j=0;j<get_number_of_columns();j++) {
        o<<(*this)(i,j)<<" ";
      }
      o<<std::endl;
    }
  }
  //! Physicial access to the elements of the matrix
  /**
   * \param[in] j physical row to access
   * \param[in] i physical Column to access
   */
  T& physical_get(const int j,const int i) const {
    if (0<=j && j<get_number_of_rows() && 0<=i && i<get_number_of_columns()) {
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
   * \param[in] val the value to set
   */
  void physical_set(const int j,const int i,const T& val) {
    if (0<=j && j<get_number_of_rows() && 0<=i && i < get_number_of_columns()) {
      (*this)(j+this->get_start(0),i+this->get_start(1))=val;
    } else {
      String msg = "Matri2D::physical_set: index out of range." ;
      throw ValueException(msg.c_str());
    }
  }

  //! Pad with a given value. Padding is defined as doubling the size
  //! in each dimension and fill the new values with the value.
  /**
    \param[out] padded the output MultiArray
    \param[in] val the value to pad with
  **/
  void pad(This& padded,T val) {
    padded.resize(2*this->get_size(0),2*this->get_size(1));
    // Copy values
    padded.fill_with_value(val);
    std::vector<index> idx(2),idx_for_padded(2);
    while (internal::roll_inds(idx, this->shape(),this->index_bases())) {
      for(int i=0;i<2;i++) {
        idx_for_padded[i]=idx[i]+(int)this->get_size(i)/2;
      }
      padded(idx_for_padded)=(*this)(idx);
    }
  }

  //! Pad the MultiArray. Padding is defined as doubling the size
  //! in each dimension and fill the new values with the previous average value.
  /**
    \param[in] padded the MultiArray padded
  **/
  void pad(This& padded) {
    double avg = this->compute_avg();
    this->pad(padded,avg);
  }

  //! Cast values
  template<typename T1>
  void cast_values(Matrix2D<T1> &out) {
    out.resize(*this);
    for(unsigned int i=0; i<this->num_elements(); i++) {
      out.data()[i] = (T1)this->data()[i];
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
    MA2::copy((MA2 &)v);
  }

  //! Sum operator
  /**
   * \param[in] v The matrix to add
   */
  This operator+(const This& v)
  const {
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

  //! Multiplication operator (element by element)
  This operator*(const This& v) const {
    This result;
    result.resize(*this);
    internal::operate_arrays(*this, v, result, '*');
    return result;
  }

  //! Division operator (element by element)
  This operator/(const This& v)
  const {
    This result;
    result.resize(*this);
    internal::operate_arrays(*this, v, result, '/');
    return result;
  }

  //! Sub operator for an array and a scalar
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

  //! Subtraction operator
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
  friend This operator*(const T& X,const This& a1) {
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

  //! Determinant (only for 2x2)
  double det() const {
    IMP_assert(get_number_of_rows() == 2,"works only for 2x2 matrices");
    IMP_assert(get_number_of_columns() == 2,"works only for 2x2 matrices");
    return (double)(physical_get(0,0)*physical_get(1,1) -
                    physical_get(1,0)*physical_get(0,1));
  }

  bool is_square() const {
    return (get_number_of_rows() == get_number_of_columns());
  }

protected:
};


IMPALGEBRA_END_NAMESPACE

// #undef DEBUG

#endif  /* IMPALGEBRA_MATRIX_2D_H */
