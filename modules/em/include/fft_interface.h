/**
 *  \file fft_interface.h
 *  \brief operations implying FFT
 *  \author Javier Velazquez-Muriel
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
*/

#ifndef IMPEM_FFT_INTERFACE_H
#define IMPEM_FFT_INTERFACE_H

#include "config.h"

IMPEM_BEGIN_NAMESPACE

//! FFT of a 2D matrix of real numbers
/**
  \param[in] m the matrix to transform
  \param[out] f the result matrix (explain the format)
  \param[out] inverse if True, the inverse FFT is computed
  \todo Under implementation
**/
template<typename T>
fft_real2d(const Matrix2D<T>& m,Matrix2D<double> &f,bool inverse=false) {

}

IMPEM_END_NAMESPACE

#endif  /* IMPEM_FFT_INTERFACE_H */
