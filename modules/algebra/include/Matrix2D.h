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
#include <IMP/algebra/internal/tnt_array2d.h>
#include <IMP/algebra/internal/jama_svd.h>
#include <IMP/algebra/internal/jama_lu.h>

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
    unsigned int nr,nc;
    nr = get_number_of_rows();
    nc = get_number_of_columns();
    for(int i=0;i<nr;i++) {
      for(int j=0;j<nc;j++) {
        (*this)(i,j)=0.;
      }
    }
  }
  //! Set the matrix to be an indentity matrix.
  void set_identity() {
    IMP_assert(is_square(), "the matrix must be square");
    set_zero();
    for (unsigned int i = 0; i < get_number_of_rows(); i++) {
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

  //! Performs bilinear interpolation for a point using the
  //! 4 closest values in the matrix
  /**
    \param[in] idx must be a class supporting access via []. It must have 2
               elements.
    \param[in] wrap if true, the image is wrapped and values from the right are
               used when the left limit is excedeed, and viceversa. Same thing
               is done between top and bottom.
    \param[in] outside Value to apply if the requested idx falls outside the
               limits of the matrix. (It is never used if wrap is requested)
  **/
  template<typename T1>
  T bilinear_interp(T1 idx,bool wrap = false,T outside = 0.0) const {
    // lower limits (xlow,ylow) are stored in v1, upper limits (xup,yup) in v2
    int v1[2], v2[2];
    double diff[2], result;

    if(!wrap) {
      // No interpolation can be done, the value is not in the image
      if(!this->is_logical_element(idx)) {
        return (T)outside;
      }
      else {
        // Set the coordinates for the 4 points used in the interpolation
        for(int i=0;i<2;i++) {
          v1[i] = (int)floor(idx[i]);
          // no interpolation can be done, v1[i] is in the border of the image
          if(v1[i]==this->get_finish(i)) {
            return (T)outside;
          } else {
            v2[i] = v1[i]+1;
          }
          diff[i] = (double)idx[i] - (double)v1[i];
        }
      }
    // Wrap is required
    } else {
      for(int i=0;i<2;i++) {
        v1[i] = (int)floor(idx[i]);
        v2[i] = v1[i]+1;
        int size = this->get_size(i);
        // this line must be before the wrapping ones
        diff[i] = (double)idx[i] - (double)v1[i];
        // wrap
        if(v1[i]<this->get_start(i) ) { v1[i]+=size; }
        if(v2[i]<this->get_start(i) ) { v2[i]+=size; }
        if(v1[i]>this->get_finish(i)) { v1[i]-=size; }
        if(v2[i]>this->get_finish(i)) { v2[i]-=size; }
      }
    }

    // Interpolate
    result= (*this)(v1[0],v1[1])*(1-diff[0])*(1-diff[1]) +
            (*this)(v2[0],v1[1])*(  diff[0])*(1-diff[1]) +
            (*this)(v1[0],v2[1])*(1-diff[0])*(diff[1])   +
            (*this)(v2[0],v2[1])*(  diff[0])*(diff[1]);

#ifdef DEBUG
    if(result>3) {
      std::cout << " v1 " << v1[0] << " " << v1[1]
                << " v2 " << v2[0] << " " << v2[1]
                << " diff " << diff[0] << " " << diff[1]
                << " dix " << idx[0] << " " << idx[1] << std::endl;
    }
#endif
    return (T)result;
  }

protected:
};


IMPALGEBRA_END_NAMESPACE

// #undef DEBUG

#endif  /* IMPALGEBRA_MATRIX_2D_H */
