/**
 *  \file MultiArray.h
 *  \brief Management of arrays of multiple dimensions
 *  \author Javier Velazquez-Muriel
 *  Copyright 2007-8 Sali Lab. All rights reserved.
*/

#ifndef IMPALGEBRA_MULTI_ARRAY_H
#define IMPALGEBRA_MULTI_ARRAY_H

#include "config.h"
#include "IMP/base_types.h"
#include "IMP/exception.h"
#include "IMP/random.h"
#include "internal/output_helpers.h"
#include "IMP/algebra/utility.h"
#include "IMP/algebra/internal/multi_array_helpers.h"
#include <ctime>

IMPALGEBRA_BEGIN_NAMESPACE




//! Template class for managing multidimensional arrays
/**
 * This class is based on boost multi_array and adds new functionality.
 */
template<typename T, int D>
class MultiArray
#ifndef SWIG
: public boost::multi_array<T, D>
#endif
{
public:
  typedef boost::multi_array_types::index index;
  typedef boost::multi_array_types::size_type size_type;
  typedef boost::multi_array<T, D> BMA;
  typedef IMP::algebra::MultiArray<T, D> This;
public:
  //! Empty constructor
  MultiArray() : BMA() {
  }

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

  //! Another way of asking for the final value (logical) for the index of the
  //! dimension
  int get_finish(const int dim) const {
    return this->index_bases()[dim] + this->shape()[dim] - 1;
  }

  //! Check if the array has some dimensionality or is just empty
  bool is_void() const {
    for (index i = 0;i < D;i++) {
      if (get_size(i) != 0) {
        return false;
      }
    }
    return true;
  }

  //! All the values of the array are set to zero
  void init_zeros() {
    std::vector<int> idx(D);
    while (internal::roll_inds(idx, this->shape(),
                                        this->index_bases())) {
      (*this)(idx) = 0;
    }
  }

  //! Returns the first element
  const T& first_element() const {
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
   * \return true if both arrays have the same size and index bases
   */
  template <typename T1>
  bool same_shape(const IMP::algebra::MultiArray<T1, D>& b) const {
    for (index i = 0;i < D;i++) {
      if (get_size(i) != b.get_size(i) || get_start(i) != b.get_start(i)) {
        return false; // different shape
      }
    }
    return true;
  }

  //! Maximum of the values in the array
  T compute_max() const {
    std::vector<index> idx(D);
    T maxval = first_element();
    while (internal::roll_inds(idx, this->shape(),
                                        this->index_bases())) {
      if ((*this)(idx) > maxval) {
        maxval = (*this)(idx);
      }
    }
    return maxval;
  }

  //! Minimum of the values in the array
  T compute_min() const {
    std::vector<index> idx(D);
    T minval = first_element();
    while (internal::roll_inds(idx, this->shape(),
                                        this->index_bases())) {
      if ((*this)(idx) < minval) {
        minval = (*this)(idx);
      }
    }
    return minval;
  }


  //! Average of the values in the array.
  /**
   * The returned value is always double, independently of the type of the
   * array.
   //  */
  double compute_avg() const {
    if (this->num_elements() <= 0) {
      return 0;
    }
    double avg = 0;
    std::vector<index> idx(D);
    while (internal::roll_inds(idx, this->shape(),
                                        this->index_bases())) {
      avg += static_cast<double>((*this)(idx));
    }
    return avg / (this->num_elements());
  }

  /** Standard deviation of the values in the array.
   * The returned value is always double, independently of the type of the
   * array.
   // */
  double compute_stddev() const {
    size_type n_elem = this->num_elements();
    if (n_elem <= 1) {
      return 0;
    }
    T val;
    double avg = 0, stddev = 0;
    std::vector<index> idx(D);
    while (internal::roll_inds(idx, this->shape(),
                                        this->index_bases())) {
      val = (*this)(idx);
      avg += static_cast<double>(val);
      stddev += static_cast<double>(val) * static_cast<double>(val);
    }
    avg /= n_elem;
    stddev = stddev / n_elem - avg * avg;
    stddev *= n_elem / (n_elem - 1);
    // Foreseeing numerical instabilities
    stddev = sqrt(static_cast<double>(std::abs(stddev)));
    return stddev;
  }

  //! Compute statistics.
  /**
   * The average, standard deviation, minimum and maximum value are
   * returned.
   */
  void compute_stats(double& avg, double& stddev, T& minval, T& maxval) const {
    if (is_void()) {
      avg = stddev = minval = maxval = 0;
    } else {
      T val = first_element();
      maxval = minval = val;

      std::vector<index> idx(D);
      while (internal::roll_inds(idx, this->shape(),
                                          this->index_bases())) {
        val = (*this)(idx);
        if (val > maxval) {
          maxval = val;
        }
        if (val < minval) {
          minval = val;
        }
        avg += static_cast<double>(val);
        stddev += static_cast<double>(val) * static_cast<double>(val);
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
    std::vector<index> idx(D);
    while (internal::roll_inds(idx, this->shape(),
                                        this->index_bases())) {
      sum += (*this)(idx);
    }
    return sum;
  }

  //! Computes the sum of all the squared elements of the array.
  //! (Frobenius norm)
  T sum_squared_elements() const {
    T sum = 0;
    std::vector<index> idx(D);
    while (internal::roll_inds(idx, this->shape(),
                                        this->index_bases())) {
      sum += (*this)(idx) * (*this)(idx);
    }
    return sum;
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
void read_binary(const std::string& filename) {
    std::ifstream in;
    in.open(filename.c_str(), std::ios::in | std::ios::binary);
    if (!in) {
      String msg = "MultiArray::read_binary: File " + filename + " not found" ;
      throw ErrorException(msg.c_str());
    }
    read_binary(in);
    in.close();
  }


  //! Read from a input stream in binary mode
  /**
   * The array must be previously resized to the correct size.
   */
  void read_binary(std::ifstream& in) {
    std::vector<index> idx(D);
    while (internal::roll_inds(idx, this->shape(),
                                        this->index_bases())) {
      in.read(reinterpret_cast< char* >(&((*this)(idx))), sizeof(T));
    }
  }


  //! Write to an ASCII file.
void write(const std::string& filename) const {
    std::ofstream out;
    out.open(filename.c_str(), std::ios::out);
    if (!out) {
      String msg = "MultiArray::write: File " + filename +
                   " cannot be opened for output" ;
      throw ErrorException(msg.c_str());
    }
    out << *this;
    out.close();
  }

  //! Write to a binary file.
void write_binary(const std::string& filename) {
    std::ofstream out;
    out.open(filename.c_str(), std::ios::out | std::ios::binary);
    if (!out) {
      String msg = "MultiArray::write: File " + filename +
                   " cannot be opened for output" ;
      throw ErrorException(msg.c_str());
    }
    write_binary(out);
    out.close();
  }

  //! Write to a output stream in binary mode.
  void write_binary(std::ofstream& out) {
    std::vector<index> idx(D);
    while (internal::roll_inds(idx, this->shape(),
                                        this->index_bases())) {
      out.write(reinterpret_cast< char* >(&((*this)(idx))), sizeof(T));
    }
  }


  friend std::istream& operator>>(std::istream& in,This& v) {
    std::vector<index> idx(D);
    while (internal::roll_inds(idx, v.shape(), v.index_bases())) {
      in >> v(idx);
    }
    return in;
  }

protected:

}; // MultiArray

//! write to an output stream for 3 dimensions
template<typename T, int D>
std::ostream& operator<<(std::ostream& ostrm,
                         const IMP::algebra::MultiArray<T, D>& v)
{
  typedef boost::multi_array_types::index index;
  std::vector<index> idx(D);

  if (v.is_void()) {
    ostrm << "NULL Array\n";
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
          idx[0] = k;
          idx[1] = j;
          idx[2] = i;
          ostrm << internal::float_to_string((double)v(idx), 10, prec) << ' ';
        }
        ostrm << std::endl;
      }
    }
  } else if (D == 2) {
    for (index j = (v).get_start(0);j <= (v).get_finish(0);j++) {
      for (index i = (v).get_start(1);i <= (v).get_finish(1);i++) {
        idx[0] = j;
        idx[1] = i;
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
  IMP::algebra::MultiArray<T, D> operator+(const T& X,
                            const IMP::algebra::MultiArray<T, D>& a1) {
    IMP::algebra::MultiArray<T, D> result(a1->shape());
    internal::operate_scalar_and_array(X, a1, result, '+');
    return result;
  }

  //! Minus operator for a scalar and an array
template <class T, int D>
  IMP::algebra::MultiArray<T, D> operator-(const T& X,
                          const IMP::algebra::MultiArray<T, D>& a1) {
    IMP::algebra::MultiArray<T, D> result(a1->shape());
    internal::operate_scalar_and_array(X, a1, result, '-');
    return result;
  }

  //! Multiplication operator for a scalar and an array
template <class T, int D>
  IMP::algebra::MultiArray<T, D> operator*(const T& X,
                          const IMP::algebra::MultiArray<T, D>& a1) {
    IMP::algebra::MultiArray<T, D> result(a1->shape());
    internal::operate_scalar_and_array(X, a1, result, '*');
    return result;
  }

  //! Division operator for a scalar and an array
template <class T, int D>
  IMP::algebra::MultiArray<T, D> operator/(const T& X,
                          const IMP::algebra::MultiArray<T, D>& a1) {
    IMP::algebra::MultiArray<T, D> result(a1->shape());
    internal::operate_scalar_and_array(X, a1, result, '/');
    return result;
  }
#endif

IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_MULTI_ARRAY_H */
