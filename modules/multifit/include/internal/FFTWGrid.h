/**
 *  \file FFTWGrid.h   \brief C++ smart pointer wrapper to fftw_malloc/free
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_FFTW_GRID_H
#define IMPMULTIFIT_FFTW_GRID_H

#include <IMP/multifit/multifit_config.h>
#include "fftw3.h"
#include <boost/noncopyable.hpp>

IMPMULTIFIT_BEGIN_INTERNAL_NAMESPACE

template <typename T>
class FFTWGrid : public boost::noncopyable {
 private:
  T *data_;

 public:
  FFTWGrid() : data_(nullptr) {}

  explicit FFTWGrid(size_t n) : data_(nullptr) { resize(n); }

  ~FFTWGrid() { release(); }

  void release() {
    if (data_) fftw_free(data_);
    data_ = nullptr;
  }

  void resize(size_t n) {
    release();
    data_ = (T *)fftw_malloc(n * sizeof(T));
    IMP_INTERNAL_CHECK(data_ != nullptr, "FFTW grid memory allocation failure");
  }

  // Get the raw pointer
  operator T *() const { return data_; }
};

IMPMULTIFIT_END_INTERNAL_NAMESPACE

#endif /* IMPMULTIFIT_FFTW_GRID_H */
