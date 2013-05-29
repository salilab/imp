/**
 *  \file multi_array_helpers.h    \brief helpers for multi_array.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_INTERNAL_MULTI_ARRAY_HELPERS_H
#define IMPALGEBRA_INTERNAL_MULTI_ARRAY_HELPERS_H

#include <IMP/algebra/algebra_config.h>
#include "boost/array.hpp"
#include "boost/multi_array.hpp"

IMPALGEBRA_BEGIN_NAMESPACE
template <typename T, int D> class MultiArray;
IMPALGEBRA_END_NAMESPACE
IMPALGEBRA_BEGIN_INTERNAL_NAMESPACE

//! Return the next set of indices within an array of given shape
/**
 * The indices are increased from last to first, and the increment is always +1
 * If the last set of indices has been reached, false is returned.
 * \param[in] inds class with the list of indices to roll. It must be a class
 * admiting access via [], and the function .size()
 * \param[in] dims class with the list of sizes the array for each dimension
 * \param[in] inds class with the list of starting indices for the array
 */
template <typename T1, typename T2, typename T3>
inline bool roll_inds(T1& inds, T2* dims, T3* start) {
  static bool initialized = false;
  int i = inds.size() - 1;
  if (initialized == false) {
    // initialize the class and check that some dimension is not 0.
    bool aux = false;
    for (unsigned int j = 0; j < inds.size(); j++) {
      inds[j] = start[j];
      if (dims[j] > 0) aux = true;
    }
    initialized = true;
    return aux;
  }
  while (i >= 0) {
    if (inds[i] < (static_cast<int>(start[i] + dims[i]) - 1)) {
      inds[i] += 1;
      return true;
    }
        // first index
        else if (i == 0) {
      initialized = false;
      return false;
    }
        // decrease one index
        else {
      inds[i] = start[i];
      i -= 1;
    }
  }
  initialized = false;
  return false;
}

//! Operates with two arrays of the same shape on a element per element basis.
template <typename T, int D>
inline void operate_arrays(const MultiArray<T, D>& a1,
                           const MultiArray<T, D>& a2, MultiArray<T, D>& result,
                           const char operation) {
  if (!(a1.same_shape(a2) && a1.same_shape(result))) {
    String op(&operation);
    IMP_FAILURE("operate_arrays:: Operator " + op +
                " not supported with arrays "
                "of different shape size and origin).");
  }
  for (unsigned long i = 0; i < a1.num_elements(); i++) {
    switch (operation) {
      case '+':
        result.data()[i] = a1.data()[i] + a2.data()[i];
        break;
      case '-':
        result.data()[i] = a1.data()[i] - a2.data()[i];
        break;
      case '*':
        result.data()[i] = a1.data()[i] * a2.data()[i];
        break;
      case '/':
        result.data()[i] = a1.data()[i] / a2.data()[i];
        break;
    }
  }
}

//! Operates with one array and a scalar on a element per element basis.
template <typename T, int D>
inline void operate_array_and_scalar(const MultiArray<T, D>& a1, const T& X,
                                     MultiArray<T, D>& result,
                                     const char operation) {
  for (unsigned long i = 0; i < a1.num_elements(); i++) {
    switch (operation) {
      case '+':
        result.data()[i] = a1.data()[i] + X;
        break;
      case '-':
        result.data()[i] = a1.data()[i] - X;
        break;
      case '*':
        result.data()[i] = a1.data()[i] * X;
        break;
      case '/':
        result.data()[i] = a1.data()[i] / X;
        break;
    }
  }
}

//! Operates with one scalar and an array on a element per element basis.
template <typename T, int D>
inline void operate_scalar_and_array(const T& X, const MultiArray<T, D>& a1,
                                     MultiArray<T, D>& result,
                                     const char operation) {

  for (unsigned long i = 0; i < a1.num_elements(); i++) {
    switch (operation) {
      case '+':
        result.data()[i] = X + a1.data()[i];
        break;
      case '-':
        result.data()[i] = X - a1.data()[i];
        break;
      case '*':
        result.data()[i] = X * a1.data()[i];
        break;
      case '/':
        result.data()[i] = X / a1.data()[i];
        break;
    }
  }
}

IMPALGEBRA_END_INTERNAL_NAMESPACE

#endif /* IMPALGEBRA_INTERNAL_MULTI_ARRAY_HELPERS_H */
