/**
 *  \file fft_operations.h
 *  \brief operations implying FFT
 *  \author Javier Velazquez-Muriel
 *  Copyright 2007-9 Sali Lab. All rights reserved.
*/

#ifndef IMPEM_FFT_OPERATIONS_H
#define IMPEM_FFT_OPERATIONS_H

#include "config.h"

IMPEM_BEGIN_NAMESPACE



//! Returns the correlation matrix between two 2D matrices using FFT
/**
  \param[in] m1 first matrix
  \param[in] m2 second matrix
  \param[out] corr matrix of results
  \todo Under implementation
**/
template<typename T>
void correlation_matrix2d(const Matrix2D<T>& m1,const Matrix2D<T>& m2,
                          Matrix2D<double>& corr) {

};


IMPEM_END_NAMESPACE

#endif  /* IMPEM_FFT_OPERATIONS_H */
