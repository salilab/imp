/**
 *  \file MultiArray.h
 *  \brief Management of arrays of multiple dimensions
 *  \author Javier Velazquez-Muriel
 *  Copyright 2007-9 Sali Lab. All rights reserved.
*/

#ifndef IMPALGEBRA_MULTI_ARRAY_H
#define IMPALGEBRA_MULTI_ARRAY_H

#include "config.h"
#include "endian.h"
#include "IMP/base_types.h"
#include "IMP/exception.h"
#include "IMP/random.h"
#include "internal/output_helpers.h"
#include "IMP/algebra/utility.h"
#include "IMP/algebra/endian.h"
#include "IMP/algebra/internal/multi_array_helpers.h"
#include "IMP/algebra/VectorD.h"
#include <ctime>
#include "boost/version.hpp"

IMPALGEBRA_BEGIN_NAMESPACE

//! Template class for managing multidimensional arrays. This class is based on
//! boost multi_array.
/**
  The class adds new functionality to the boost class:
    1) Operators + - * and / to deal with other multiarrays and scalars
    2) Computation of maximum, minimum, and statistical properties of the
       array.
    3) Read/Write both in text and binary modes (taking into account the
       endianess).
**/
template<typename T, int D>
class MultiArray
#ifndef SWIG
  : public boost::multi_array<T, D>
#endif
{
public:
  typedef boost::multi_array_types::index index;
  typedef boost::multi_array_types::size_type size_type;
//  typedef boost::multi_array_types::extent_gen extent_gen;
  typedef boost::multi_array<T, D> BMA;
  typedef MultiArray<T, D> This;

public:
  //! Empty constructor
  MultiArray() : BMA() {}

  //! Another way of asking for the size of a given dimension. You can always
  //! use x.shape()[i] too.
  int get_size(const int dim) const {
    return this->shape()[dim];
  }

  //! Another way of asking for the initial value (logical)
  //! for the index of the dimension. You can always use index_bases()[dim]
  int get_start(const int dim) const {
    return this->index_bases()[dim];
  }

  //! Another way setting the initial value (logical)
  //! for the index of the dimension. You can always use reindex()
  void set_start(const int dim,const int value) {
    std::vector<typename boost::multi_array_types::index> idx(D);
    for (unsigned int i = 0;i < D;i++) {
      idx[i] = this->index_bases()[i];
    }
    idx[dim] = value;
    this->reindex(idx);
  }

  //! Another way setting the initial value (logical)
  //! for the index of the dimension.
  /**
   * \param[in] v Any class able to be accessed with []
   */
  template<typename T1>
  void set_start(const T1& v) {
    this->reindex(v);
  }

  //! Sets the zero (0,0,0,...) of the logical coordinates at the center of
  //! the center of the matrix.
  /**
   * Some examples:
   *
   * \code
   * m.centered_start(); // if m is a 5x4 matrix, the origin will be (-2,-2)
   * m.centered_start(); // if m is a 16x3 matrix, the origin will be (-8,-1)
   * \endcode
   */
  void centered_start() {
    std::vector<int> idx(D);
    for (unsigned int i = 0;i < D;i++) {
      idx[i] =(-1) * (int)this->shape()[i]/2;
    }
    this->reindex(idx);
  }

  //! Another way of asking for the final value (logical) for the index of the
  //! dimension
  int get_finish(const int dim) const {
    return (this->index_bases()[dim] + this->shape()[dim] - 1);
  }

  //! Check if the array has some dimensionality or is just empty
  bool is_void() const {
    for (index i = 0;i < D;i++) {
      if ((index)get_size(i) != 0) {
        return false;
      }
    }
    return true;
  }

  //! All the values of the array are set to zero
  void init_zeros() {
    for (unsigned long i=0;i<this->num_elements() ;i++) {
      this->data()[i] = 0;
    }
  }

  //! Fill all the voxels of the array with a given value
  void fill_with_value(T val=0) {
    for (unsigned long i=0;i<this->num_elements() ;i++) {
      this->data()[i] = val;
    }
  }

  //! Copies the contents of a MultiArray to this one (no resizing is done).
  //! For copying with resizing use operator=
  /**
   * \param[in] v MultiArray whose contents to copy
   */
  void copy(This &v) {
    for (unsigned long i=0;i<this->num_elements() ;i++) {
      this->data()[i] = v.data()[i];
    }
  }

  //! Returns true if the LOGICAL index belongs to those of the matrix
  /**
   * \param[in] v Any class able to be accessed with []
   */
  template<typename T1>
  bool is_logical_element(T1 &v) const {
    for (unsigned int i = 0;i < D;i++) {
      if(v[i]<this->get_start(i) || this->get_finish(i)<v[i]) {
        return false;
      }
    }
    return true;
  }

  //! Returns true if the PHYSICAL index belongs to those of the matrix
  /**
   * \param[in] v Any class able to be accessed with []
   */
  template<typename T1>
  bool is_physical_element(T1 &v) const {
    for (unsigned int i = 0;i < D;i++) {
      if(v[i]<0 || (this->get_size(i)-1)<v[i]) {
        return false;
      }
    }
    return true;
  }

  //! Returns the first element
  bool first_element() const {
    std::vector<index> idx(D);
    for (unsigned int i = 0;i < D;i++) {
      idx[i] = this->index_bases()[i];
    }
    return (*this)(idx);
  }


#ifndef SWIG

  //! Sum operator
  This operator+(const This& v) const {
    This result(v);
    internal::operate_arrays(*this, v, result, '+');
    return result;
  }

  //! Minus operator
  This operator-(const This& v) const {
    This result(v);
    internal::operate_arrays(*this, v, result, '-');
    return result;
  }

  //! Multiplication operator
  This operator*(const This& v) const {
    This result(v);
    internal::operate_arrays(*this, v, result, '*');
    return result;
  }

  //! Division operator
  This operator/(const This& v) const {
    This result(v);
    internal::operate_arrays(*this, v, result, '/');
    return result;
  }

  //! Addition operator
  void operator+=(const This& v) const {
    internal::operate_arrays(*this, v, *this, '+');
  }

  //! Substraction operator
  void operator-=(const This& v) const {
    internal::operate_arrays(*this, v, *this, '-');
  }

  //! Multiplication operator
  void operator*=(const This& v) const {
    internal::operate_arrays(*this, v, *this, '*');
  }

  //! Division operator
  void operator/=(const This& v) const {
    internal::operate_arrays(*this, v, *this, '/');
  }

  //! Sum operator for an array and a scalar
  This operator+(const T& v) const {
    This result(v);
    internal::operate_array_and_scalar(*this, v, result, '+');
    return result;
  }

  //! Minus operator for an array and a scalar
  This operator-(const T& v) const {
    This result(v);
    internal::operate_array_and_scalar(*this, v, result, '-');
    return result;
  }

  //! Multiplication operator for an array and a scalar
  This operator*(const T& v) const {
    This result(v);
    internal::operate_array_and_scalar(*this, v, result, '*');
    return result;
  }


  //! Division operator for an array and a scalar
  This operator/(const T& v) const {
    This result(v);
    internal::operate_array_and_scalar(*this, v, result, '/');
    return result;
  }

  //! Addition operator for an array and a scalar
  void operator+=(const T& v) const {
    internal::operate_array_and_scalar(*this, v, *this, '+');
  }

  //! Substraction operator for an array and a scalar
  void operator-=(const T& v) const {
    internal::operate_array_and_scalar(*this, v, *this, '-');
  }

  //! Multiplication operator for an array and a scalar
  void operator*=(const T& v) const {
    internal::operate_array_and_scalar(*this, v, *this, '*');
  }

  //! Division operator for an array and a scalar
  void operator/=(const T& v) const {
    internal::operate_array_and_scalar(*this, v, *this, '/');
  }
#endif

  //! Print shape of multidimensional array.
  /**
   * This function shows the size, starting and finishing indexes of the
   * given array. No end of line is printed neither at the beginning nor
   * the end.
   */
  void print_shape(std::ostream& out = std::cout) const {
    out << "Dimensions: " << D << std::endl;
    out << "Size and range of each of the dimensions: " << std::endl;
    for (index i = 0;i < D;i++) {
      out << "Size: " << get_size(i) << " range: "
      << get_start(i) << " .. " << get_finish(i) << std::endl;
    }
  }

  //! Compares the shape of two multidimensional arrays
  /**
   * \return true if both arrays have the same size and origin
   */
  template <typename T1>
  bool same_shape(const MultiArray<T1, D>& b) const {
    for (index i = 0;i < D;i++) {
      if (get_size(i) != b.get_size(i) || get_start(i) != b.get_start(i)) {
        return false; // different shape
      }
    }
    return true;
  }


  //! Compares the size of two multidimensional arrays
  /**
   * \return true if both arrays have the same size
   */
  template <typename T1>
  bool same_size(const MultiArray<T1, D>& b) const {
    for (index i = 0;i < D;i++) {
      if (get_size(i) != b.get_size(i)) { return false; }
    }
    return true;
  }

  //! Compares the origin of two multidimensional arrays
  /**
   * \return true if both arrays have the same origin
   */
  template <typename T1>
  bool same_start(const MultiArray<T1, D>& b) const {
    for (index i = 0;i < D;i++) {
      if (get_start(i) != b.get_start(i)) {return false;}
    }
    return true;
  }

  //! Maximum of the values in the array
  T compute_max() const {
    T maxval = first_element();
    for(unsigned long i = 0; i < this->num_elements();i++) {
      if(this->data()[i] > maxval) {
        maxval = this->data()[i];
      }
    }
    return maxval;
  }

  //! Maximum of the values in the array
  /**
    \param[out] max_idx index containing the maximum value
  **/
  template<typename T1>
  T compute_max(T1& max_idx) const {
    std::vector<index> idx(D);
    T maxval = first_element();
    while (internal::roll_inds(idx, this->shape(),this->index_bases())) {
      if ((*this)(idx) > maxval) {
        maxval = (*this)(idx);
        for(int i=0;i<D;i++) {
          max_idx[i]=idx[i];
        }
      }
    }
    return maxval;
  }

  //! Minimum of the values in the array
  T compute_min() const {
    T minval = first_element();
    for(unsigned long i = 0; i < this->num_elements();i++) {
      if(this->data()[i] > minval) {
        minval = this->data()[i];
      }
    }
    return minval;
  }

  //! Minimum of the values in the array
  /**
    \param[out] min_idx index containing the minimum value
  **/
  template<typename T1>
  T compute_min(T1& min_idx) const {
    std::vector<index> idx(D);
    T minval = first_element();
    while (internal::roll_inds(idx, this->shape(),this->index_bases())) {
      if ((*this)(idx) < minval) {
        minval = (*this)(idx);
        for(int i=0;i<D;i++) {
          min_idx[i]=idx[i];
        }
      }
    }
    return minval;
  }


  //! Average of the values in the array.
  /**
   * The returned value is always double, independently of the type of the
   * array.
   **/
  double compute_avg() const {
    if (this->num_elements() == 0) { return 0; }
    double avg = 0;
    for (unsigned long i=0;i<this->num_elements() ;i++) {
      avg += static_cast<double>(this->data()[i]);
    }
    return avg / (this->num_elements());
  }

  //! Standard deviation of the values in the array.
  double compute_stddev() const {
    size_type n_elem = this->num_elements();
    if (n_elem <= 1) {
      return 0;
    }
    double avg = 0, stddev = 0,val;
    for (unsigned long i=0;i<this->num_elements() ;i++) {
      val = static_cast<double>(this->data()[i]);
      avg += val;
      stddev += val*val;
    }
    avg /= n_elem;
    stddev = stddev / n_elem - avg * avg;
    stddev *= n_elem / (n_elem - 1);
    // Foreseeing numerical instabilities
    stddev = sqrt(static_cast<double>(std::abs(stddev)));
    return stddev;
  }

  //! Compute average, standard deviation, minimum and maximum values
  void compute_stats(double& avg, double& stddev, T& minval, T& maxval) const {
    if (is_void()) {
      avg = stddev = minval = maxval = 0;
    } else {
      T val = first_element();
      avg = stddev = 0;
      maxval = minval = val;
      std::vector<index> idx(D);
      for (unsigned long i=0;i<this->num_elements() ;i++) {
        if (this->data()[i] > maxval) { maxval = val; }
        if (this->data()[i] < minval) { minval = val; }
        val = static_cast<double>(this->data()[i]);
        avg += val;
        stddev += val*val;
      }
      // Average
      size_type n_elem = this->num_elements();
      avg /= n_elem;
      if (n_elem > 1) {
        // Standard deviation
        stddev = stddev / n_elem - avg * avg;
        stddev *= n_elem / (n_elem - 1);
        // Foreseeing numerical instabilities
        stddev = sqrt(static_cast<double>(std::abs(stddev)));
      } else {
        stddev = 0;
      }
    }
  }

  //! Computes the sum of all the array elements.
  T sum_elements() const {
    T sum = 0;
    for (unsigned long i=0;i<this->num_elements();i++) {
      sum += this->data()[i];
    }
    return sum;
  }

  //! Computes the sum of all the squared elements of the array.
  //! (Frobenius norm)
  T sum_squared_elements() const {
    T sum = 0;
    for (unsigned long i=0;i<this->num_elements();i++) {
      sum += this->data()[i]*this->data()[i];
    }
    return sum;
  }


  //! Computes the sum of the squared elements of the difference MultiArray
  //! obtained from substracting v.
  /**
    \note Both MultiArrays are required to have the same shape (size and origin)
  */
  T squared_difference(const This& v) const {
    if(!this->same_shape(v)) {
      String msg = "squared_difference:: operation not supported with arrays "
                   "of different shape (size and origin).";
      throw ErrorException(msg.c_str());
    } else {
      T sum = 0,aux;
      for (unsigned long i=0;i<this->num_elements();i++) {
        aux = this->data()[i] - v.data()[i];
        sum += aux*aux;
      }
      return sum;
    }
  }


  //! Computes the cross correlation coeffcient between two MultiArrays
  /**
    \note Both MultiArrays are required to have the same size, but not the
    same origin
    \param[in] v array to compute the cross_correlation with.
    \param[in] apply_threshold true if a threshold is applied to the elements
               of v .
    \param[in] threshold minimum value for an element v to consider it in the
               computation .
    \param[in] divide_by_stddev true if the cross correlation term is divided
               by the standard deviation to get the cross correlation
               coefficient (0 <= ccc <= 1).
    \param[in] force_recalc_stats true if the statistics (mean, stddev) for the
               Multiarrays must be recalculated (default).
               If the statistics are known from previous computations,
               you can speed up the next computations setting this variable
               to false and directly providing the parameters.
    \param[in] avg average of this Multiarray.
    \param[in] stddev standard deviation of this Multiarray.
    \param[in] avg_v average of Multiarray v.
    \param[in] stddev_v standard deviation of Multiarray v.

  */
  double cross_correlation_coefficient(const This& v,
                                      bool apply_threshold=false,
                                      double threshold=0.0,
                                      bool divide_by_stddev=true,
                                      bool force_recalc_stats=true,
                                      double avg=0.0,
                                      double stddev=0.0,
                                      double avg_v=0.0,
                                      double stddev_v=0.0) {
    if(this->same_size(v)==false) {
      String msg = "cross_correlation_coefficient:: operation "
                   "not supported with arrays of different size. ";
      throw ErrorException(msg.c_str());
    }

    double t_avg,t_stddev,v_avg,v_stddev;
    T t_max,t_min,v_max,v_min;

    if(force_recalc_stats) {
      this->compute_stats(t_avg,t_stddev,t_min,t_max);
      v.compute_stats(v_avg,v_stddev,v_min,v_max);
    } else {
      t_avg=avg;
      t_stddev=stddev;
      v_avg=avg_v;
      v_stddev=stddev_v;
    }

    // Get number of elements
    double n_elems = 1.0;
    for (unsigned int i = 0;i < D;i++) {
      n_elems *= static_cast<double>(this->get_size(i));
    }

    double epsilon = 1e-6;
    // Check for stddevs near zero using the default tolerance
    if (divide_by_stddev && ( almost_equal(t_stddev,0.0,epsilon) ||
                              almost_equal(v_stddev,0.0,epsilon))) {
      return 0.0;
    }

    double ccc = 0.0;
    std::vector<index> idx(D);

    // Fast version
    if(this->same_start(v)) {
      while (internal::roll_inds(idx, this->shape(), this->index_bases())) {
        if(!apply_threshold || (apply_threshold && v(idx) > threshold)) {
          ccc += (*this)(idx)*v(idx);
        }
      }
    } else { // Different origins
      while (internal::roll_inds(idx, this->shape(), this->index_bases())) {
        // Check if the index belongs to v
        if(v.is_logical_element(idx)) {
          // Check for threshold
          if(!apply_threshold || (apply_threshold && v(idx) > threshold)) {
            ccc += (*this)(idx)*v(idx);
          }
        }
      }
    }
    if (divide_by_stddev) {
      ccc = (ccc-n_elems*t_avg*v_avg)/(n_elems*t_stddev*v_stddev);
    }
    return ccc;
  }


  //! Pad the MultiArray. Padding is defined as doubling the size
  //! in each dimension and fill the new values with the previous average value.
  /**
    \param[in] padded the MultiArray padded
  **/
//  void pad(This& padded) {
//    double avg = this->compute_avg();
//    this->pad(padded,avg);
//  }

  //! Pad with a given value (only works for boost 1.37 and higher)
  /**
    \param[out] padded the output MultiArray
    \param[in] val the value to pad with
  **/
//  void pad(This& padded,T val) {
//    std::vector<int> extents(D);
//    for(int i=0;i<D;i++) {
//      extents[i]=2*this->get_size(i);
//    }
//    padded.resize(extents);
//    // Copy values
//    padded.fill_with_value(val);
//    std::vector<index> idx(D),idx_for_padded(D);
//    while (internal::roll_inds(idx, this->shape(),this->index_bases())) {
//      for(int i=0;i<D;i++) {
//        idx_for_padded[i]=idx[i]+(int)this->get_size(i)/2;
//      }
//      padded(idx_for_padded)=(*this)(idx);
//    }
//  }

  //! Cast values
  template<typename T1>
  void cast_values(MultiArray<T1, D> &out) {
    boost::array<int, D> shape;
    std::copy(this->shape(), this->shape() + D, shape.begin());
    out.resize(shape);
    for(unsigned int i=0; i<this->num_elements(); i++) {
      out.data()[i] = this->data()[i];
    }
  }

  //! Read from an ASCII file.
  /**
   * The array must be previously resized to the correct size.
   */
  void read(const std::string& filename) {
    std::ifstream in;
    in.open(filename.c_str(), std::ios::in);
    if (!in) {
      String msg = "MultiArray::read: File " + filename + " not found" ;
      throw ErrorException(msg.c_str());
    }
    in >> *this;
    in.close();
  }

  //! Read from a binary file.
  /**
   * The array must be previously resized to the correct size.
   */
  void read_binary(const std::string& filename,bool reversed=false) {
    std::ifstream in;
    in.open(filename.c_str(), std::ios::in | std::ios::binary);
    if (!in) {
      String msg = "MultiArray::read_binary: File " + filename + " not found" ;
      throw ErrorException(msg.c_str());
    }
    read_binary(in,reversed);
    in.close();
  }

  //! Read from a input stream in binary mode
  /**
   * The array must be previously resized to the correct size.
   */
  void read_binary(std::ifstream& in,bool reversed=false) {
    for (unsigned long i=0;i<this->num_elements();i++) {
      if (!reversed) {
        in.read(reinterpret_cast< char* >(&(this->data()[i])), sizeof(T));
      } else {
        reversed_read(reinterpret_cast< char* >(&(this->data()[i])),
                       sizeof(T),1,in,true);
      }
    }
  }

  //! Write to an ASCII file.
  void write(const std::string& filename) const {
    std::ofstream out;
    out.open(filename.c_str(), std::ios::out);
    if (!out) {
      String msg = "MultiArray::write: "+
                    filename+" cannot be opened for output" ;
      throw ErrorException(msg.c_str());
    }
    out << *this;
    out.close();
  }

  //! Write to a binary file.
  void write_binary(const std::string& filename,bool reversed=false) {
    std::ofstream out;
    out.open(filename.c_str(), std::ios::out | std::ios::binary);
    if (!out) {
      String msg = "MultiArray::write: " +
                  filename +" cannot be opened for output" ;
      throw ErrorException(msg.c_str());
    }
    write_binary(out,reversed);
    out.close();
  }

  //! Write to a output stream in binary mode.
  void write_binary(std::ofstream& out,bool reversed=false) {
    for (unsigned long i=0;i<this->num_elements();i++) {
      if (!reversed) {
        out.write(reinterpret_cast< char* >(&(this->data()[i])), sizeof(T));
      } else {
        reversed_write(reinterpret_cast< char* >(&(this->data()[i])),
                       sizeof(T),1,out,true);
      }
    }
  }

  friend std::istream& operator>>(std::istream& in,This& v) {
    for (unsigned long i=0;i<v.num_elements();i++) {
      in >> v.data()[i];
    }
    return in;
  }

protected:
}; // MultiArray

//! write to an output stream for 3 dimensions
template<typename T, int D>
std::ostream& operator<<(std::ostream& ostrm,
                         const MultiArray<T, D>& v)
{
  typedef boost::multi_array_types::index index;
  std::vector<index> idx(D);

  if (v.is_void()) {
    ostrm << "NULL Array" << std::endl;
    return ostrm;
  } else {
    ostrm << std::endl;
  }
  T absmax = v.first_element();
  while (internal::roll_inds(idx, v.shape(), v.index_bases())) {
    if (std::abs(v(idx)) > absmax) {
      absmax = v(idx);
    }
  }
  int prec = internal::best_precision(absmax, 10);


  // Print differently depending on dimension
  if (D == 3) {
    for (index k = (v).get_start(0);k <= (v).get_finish(0);k++) {
      if (v.get_size(0) > 1) ostrm << "Slice No. " << k << std::endl;
      for (index j = (v).get_start(1);j <= (v).get_finish(1);j++) {
        for (index i = (v).get_start(2);i <= (v).get_finish(2);i++) {
          idx[0] = k; idx[1] = j; idx[2] = i;
          ostrm << internal::float_to_string((double)v(idx), 10, prec) << ' ';
        }
        ostrm << std::endl;
      }
    }
  } else if (D == 2) {
    for (index j = (v).get_start(0);j <= (v).get_finish(0);j++) {
      for (index i = (v).get_start(1);i <= (v).get_finish(1);i++) {
        idx[0] = j; idx[1] = i;
        ostrm << internal::float_to_string((double)v(idx), 10, prec) << ' ';
      }
      ostrm << std::endl;
    }
  } else {
    while (internal::roll_inds(idx, v.shape(), v.index_bases())) {
      ostrm << internal::float_to_string((double)v(idx), 10, prec) << ' ';
    }
    ostrm << std::endl;
  }
  return ostrm;
}

#ifndef SWIG
  //! Sum operator for a scalar and an array
template <class T, int D>
  MultiArray<T, D> operator+(const T& X,
                             const MultiArray<T, D>& a1) {
    MultiArray<T, D> result(a1->shape());
    internal::operate_scalar_and_array(X,a1,result,"+");
    return result;
  }

  //! Minus operator for a scalar and an array
template <class T, int D>
  MultiArray<T, D> operator-(const T& X,
                             const MultiArray<T, D>& a1) {
    MultiArray<T, D> result(a1->shape());
    internal::operate_scalar_and_array(X,a1,result,"-");
  }

  //! Multiplication operator for a scalar and an array
template <class T, int D>
  MultiArray<T, D> operator*(const T& X,
                             const MultiArray<T, D>& a1) {
    MultiArray<T, D> result(a1->shape());
    internal::operate_scalar_and_array(X,a1,result,"*");
    return result;
  }

  //! Division operator for a scalar and an array
template <class T, int D>
  MultiArray<T, D> operator/(const T& X,
                             const MultiArray<T, D>& a1) {
    MultiArray<T, D> result(a1->shape());
    internal::operate_scalar_and_array(X,a1,result,"/");
    return result;
  }
#endif

IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_MULTI_ARRAY_H */
