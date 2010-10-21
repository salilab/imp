/**
 *  \file FFTinterface.h
 *  \brief Functions to deal with very common math operations
 *  \brief Interface between FFTW and this package.
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_FF_TINTERFACE_H
#define IMPEM2D_FF_TINTERFACE_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/em/DensityMap.h"
#include "IMP/algebra/Matrix2D.h"
#include "IMP/algebra/Matrix3D.h"
#include "IMP/exception.h"
#include <fftw3.h>
#include <complex>

IMPEM2D_BEGIN_NAMESPACE


//! Get the proper size to do fast FFT with fftw3
/**
  \note To save time of computation and memory space, the dimension of
  the FFT transform in fftw3 does not have same dimensions as the input.
  The las dimension is only floor(n/2)+1, being n the size
  of the last dimension of the input (vector, matrix, etc..)
**/
inline unsigned int get_fast_fft_size(unsigned int dim) {
  return floor(dim/2)+1;
}

//! Interface class between fftw3 and FFTs in 2D
/**
  To use, simply:
  \code
    FFT2D fft(m,M);
    fft.execute(); // after this, M contains a complex matrix with the FFT
  \endcode
  \note to save time of computation and memory space, M does not have same
  dimensions as m. The las dimension is only floor(n/2)+1, being n the size
  of the last dimension of m.
**/
class FFT2D {
public:
  FFT2D() {};
  FFT2D(algebra::Matrix2D_d &in,algebra::Matrix2D_c &out) {
    prepare(in,out);
  }

  ~FFT2D() { fftw_destroy_plan(FFT_); }

  inline void prepare(algebra::Matrix2D_d &in,
           algebra::Matrix2D_c &out) {
    // Resize the output to the size used by fftw
    out.resize(in.get_number_of_rows(),
               get_fast_fft_size(in.get_number_of_columns()));
    // Prepare plan
    FFT_ = fftw_plan_dft_r2c_2d(in.get_number_of_rows(),
          in.get_number_of_columns(),
          in.data(),
          (fftw_complex *)out.data(),
          FFTW_ESTIMATE | FFTW_PRESERVE_INPUT);
  }

  inline void execute() { fftw_execute(FFT_); }

private:
  fftw_plan FFT_;
};

//! Interface class between fftw3 and IFFTs in 2D
/**
  To use, simply:
  \code
    IFFT2D ifft;
    ifft.prepare(M,m);
    ifft.execute(); // after this, m contains a matrix of reals with
    the inverse FFT (not normalized)
  \endcode
  \note to save time of computation and memory space, M does not have same
  dimensions as m. The las dimension is only floor(n/2)+1, being n the size
  of the last dimension of m. The output m MUST have correct dimensions in
  advance.
**/
class IFFT2D {
public:
  IFFT2D() {};
  IFFT2D(algebra::Matrix2D_c &in,algebra::Matrix2D_d &out) {
    prepare(in,out);
  }
  ~IFFT2D() { fftw_destroy_plan(IFFT_); }

  inline void prepare(algebra::Matrix2D_c &in,algebra::Matrix2D_d &out) {
    // Prepare plan
    unsigned int rows=out.get_number_of_rows();
    unsigned int cols=out.get_number_of_columns();
    if(rows==0 || cols==0) {
      IMP_THROW("IFFT2D: output matrix has no rows or columns ",ValueException);
    }

    IFFT_ = fftw_plan_dft_c2r_2d(rows,cols,
              (fftw_complex *)in.data(),
              out.data(),
              FFTW_ESTIMATE);
//              FFTW_ESTIMATE | FFTW_PRESERVE_INPUT);
  }

  inline void execute() { fftw_execute(IFFT_); }

private:
  fftw_plan IFFT_;
};


//! Interface class between fftw3 and FFTs in 1D
/**
  To use, simply:
  \code
    FFT1D fft;
    fft.prepare(m,M);
    fft.execute(); // after this, M contains a matrix of complex with the FFT
  \endcode
  \note to save time of computation and memory space, the size of M is only
  floor(n/2)+1, being n the size of m.
**/
class FFT1D {
public:
  FFT1D() {
    prepared_=false;
  }

  FFT1D(std::vector<double> &in,
                          std::vector<std::complex<double> > &out) {
    prepare(in,out);
  }
  ~FFT1D() { fftw_destroy_plan(FFT_); }

  inline void prepare(std::vector<double> &in,
        std::vector<std::complex<double> > &out) {
    // Resize for fftw3
    out.resize(get_fast_fft_size(in.size()));
    // Prepare plan
      FFT_ = fftw_plan_dft_r2c_1d(in.size(),
          &in.front(),
          (fftw_complex *)(&out.front()),
          FFTW_ESTIMATE | FFTW_PRESERVE_INPUT);
    prepared_=true;
  }

  inline void execute() {
    if(!prepared_) {
      IMP_THROW("Attempt to execute a FFT without preparing it before",
                                                            IOException);
    }
    fftw_execute(FFT_);
  }

private:
  bool prepared_;
  fftw_plan FFT_;
};

//! Interface class between fftw3 and IFFTs in 1D
/**
  To use, simply:
  \code
    IFFT1D ifft(M,m);
    ifft.execute(); // after this, m contains a vector of reals with
    the inverse FFT (not normalized)
  \endcode
  \note to save time of computation and memory space, the size of M is
  only floor(n/2)+1, being n the size of m. The output m must have the proper
  size in advance
**/
class IFFT1D {
public:
  IFFT1D() {prepared_=false;};
  IFFT1D(std::vector<std::complex<double> > &in,std::vector<double> &out) {
    prepare(in,out);
  }
  ~IFFT1D() { fftw_destroy_plan(IFFT_); }

  inline void prepare(std::vector<std::complex<double> > &in,
               std::vector<double> &out) {
    // Prepare plan
    IFFT_ = fftw_plan_dft_c2r_1d(out.size(),
              (fftw_complex *)(&in.front()),
              &out.front(),
              FFTW_ESTIMATE);
//              FFTW_ESTIMATE | FFTW_PRESERVE_INPUT); // does not work
    prepared_=true;
  }

  inline void execute() {
    if(!prepared_) {
      IMP_THROW("Attempt to execute a IFFT without preparing it before",
                                                            IOException);
    }
    fftw_execute(IFFT_);
  }

private:
  bool prepared_;
  fftw_plan IFFT_;
};

//! Interface class between fftw3 and FFTs in 3D
/**
  To use, simply:
  \code
    FFT3D fft(m,M);
    fft.execute(); // after this, M contains a matrix of complex with the FFT
  \endcode
  \note to save time of computation and memory space, M does not have same
  dimensions as m. The las dimension is only floor(n/2)+1, being n the size
  of the last dimension of m.
**/
class FFT3D {
public:
  FFT3D() {};
  FFT3D(algebra::Matrix3D<double> &in,
        algebra::Matrix3D<std::complex<double> > &out) { prepare(in,out); }
  FFT3D(em::DensityMap &in,
        algebra::Matrix3D<std::complex<double> > &out) { prepare(in,out); }
  ~FFT3D() { fftw_destroy_plan(FFT_); }

  //! Prepares the FFTW plan for Matrix3D
  inline void prepare(algebra::Matrix3D<double> &in,
                      algebra::Matrix3D<std::complex<double> > &out) {

    out.resize(in.get_size(0),
               in.get_size(1),
               get_fast_fft_size(in.get_size(2)));
    internal_prepare(in.get_size(0),in.get_size(1),in.get_size(2),
                     in.data(),out.data());
  }

  //! Prepares the FFTW plan for DensityMap
  inline void prepare(em::DensityMap &in,
                algebra::Matrix3D<std::complex<double> > &out) {
   out.resize(in.get_header()->get_nz(),
               in.get_header()->get_ny(),
               get_fast_fft_size(in.get_header()->get_nx()));
    internal_prepare(in.get_header()->get_nz(),
                     in.get_header()->get_ny(),
                     in.get_header()->get_nx(),
                     in.get_data(),out.data());
  }

  inline void internal_prepare(int n0,int n1,int n2,double *in,
                          std::complex<double> *out) {
    // Prepare plan
    FFT_ =fftw_plan_dft_r2c_3d(n0,n1,n2,in,(fftw_complex *)out,
                            FFTW_ESTIMATE | FFTW_PRESERVE_INPUT);
  }

  inline void execute() { fftw_execute(FFT_); }

private:
  fftw_plan FFT_;
};


//! Interface class between fftw3 and IFFTs in 3D
/**
  To use, simply:
  \code
    IFFT3D fft;
    fft.prepare(M,m);
    fft.execute(); // after this, M contains a matrix of reals with the
    inverse FFT (not normalized)
  \endcode
  \note to save time of computation and memory space, M does not have same
  dimensions as m. The las dimension is only floor(n/2)+1, being n the size
  of the last dimension of m. The output m must have the proper dimensions
  in advance
**/
class IFFT3D {
public:
   IFFT3D() {};
   IFFT3D(algebra::Matrix3D<std::complex<double> > &in,
                      algebra::Matrix3D<double> &out) { prepare(in,out);}
  ~IFFT3D() { fftw_destroy_plan(IFFT_); }

  //! Prepares the FFTW plan for Matrix3D
  inline void prepare(algebra::Matrix3D<std::complex<double> > &in,
                      algebra::Matrix3D<double> &out)  {
    internal_prepare(out.get_size(0),out.get_size(1),out.get_size(2),
                     in.data(),out.data());
  }

  //! Prepares the FFTW plan for DensityMap
  inline void prepare(algebra::Matrix3D<std::complex<double> > &in,
                     em::DensityMap &out) {
    internal_prepare(out.get_header()->get_nz(),
                     out.get_header()->get_ny(),
                     out.get_header()->get_nx(),
                     in.data(),out.get_data());
  }

  inline void internal_prepare(int n0,int n1,int n2,
                          std::complex<double> *in,double *out) {
    // Prepare plan
    IFFT_=fftw_plan_dft_c2r_3d(n0,n1,n2,(fftw_complex *)in, out,
               FFTW_ESTIMATE);
//              FFTW_ESTIMATE | FFTW_PRESERVE_INPUT); // does not work
  }

  inline void execute() { fftw_execute(IFFT_); }

private:
  fftw_plan IFFT_;
};


//! Generates the complete FFT2D that would require the use of a general plan
//! in fftw3 from the results of using the plan fftw_plan_dft_r2c_2d.
/**
  \note Using this function saves the use of the general plan, which is half
  of the computations. The relation between the dimensions:
    rows(in) = rows(out)
    columns(in) = floor(columns(out)/2)+1
  both matrices must have the proper dimensions before applying the function
  \link http://www.fftw.org/fftw3_doc/Multi_002ddimensional-Transforms.html
**/
template <typename T>
void symmetrize_FFT2D(algebra::Matrix2D<T> &in,algebra::Matrix2D<T> &out) {
  int ny = out.get_size(0);
  int nx = out.get_size(1);
  int h = get_fast_fft_size(nx); // Size of the last dimension if fftw
  int new_i,new_j;
  for (int i = 0;i < ny; ++i) {
    for (int j = 0;j < h;++j) {
      out(i,j)=in(i,j);
      if(i==0) {new_i=0;} else { new_i=ny-i;}
      if(j==0) {new_j=0;} else { new_j=nx-j;}
      out(new_i,new_j)=conj(out(i,j));
    }
  }
};

//! Compresses the "complete" matrix2D that the general plan in fftw3 requires
//! to the format used by the plan fftw_plan_dft_r2c_2d
/**
  \note Using this function saves the use of the general plan, thus requiring
  only half of the computations when applying IFFT2D. The relation between
  the dimensions:
    rows (out) = rows(in)
    columns(out) = floor(columns(in)/2)+1
  \note The input matrix is modified and part of the values are lost
  \link http://www.fftw.org/fftw3_doc/Multi_002ddimensional-Transforms.html
**/
template <typename T>
void desymmetrize_FFT2D(algebra::Matrix2D<T> &in) {
  in.resize(in.get_number_of_rows(),
            get_fast_fft_size(in.get_number_of_columns()));
}

//! Generates the complete FFT3D that would require the use of a general plan
//! in fftw3 from the results of using the plan fftw_plan_dft_r2c_3d.
/**
  \note This function saves using the general plan (half the computations).
  The relation between the dimensions:
    slices(in) = slices(out)
    rows(in) = rows (out)
    columns(in) = floor(columns(out)/2)+1
  Both matrices must have the proper dimensions before applying the function
  \link http://www.fftw.org/fftw3_doc/Multi_002ddimensional-Transforms.html
**/
template <typename T>
void symmetrize_FFT3D(algebra::Matrix3D<T> &in,algebra::Matrix3D<T> &out) {
  int nz = out.get_size(0),ny = out.get_size(1), nx = out.get_size(2);
  int h = get_fast_fft_size(nx); //Size of the last dimension if fftw
  int new_i,new_j,new_k;
  for (int i = 0;i < nz; ++i) {
    for (int j = 0;j < ny;++j) {
      for (int k = 0;k < h;++k) {
        out(i,j,k)=in(i,j,k);
        if(i==0) {new_i=0;} else { new_i=nz-i;}
        if(j==0) {new_j=0;} else { new_j=ny-j;}
        if(k==0) {new_k=0;} else { new_k=nx-k;}
        out(new_i,new_j,new_k)=conj(out(i,j,k));
      }
    }
  }
};

//! Compresses the "complete" matrix3D that the general plan in fftw3 requires
//! to the format used by the plan fftw_plan_dft_r2c_3d
/**
  \note Using this function saves the use of the general plan, thus requiring
  only half of the computations when applying IFFT2D. The relation between
  the dimensions:
    slices(out) = slices(in)
    rows(out) = rows(in)
    columns(out) = floor(columns(in)/2)+1
  \note The input matrix is modified and part of the values are lost
**/
template <typename T>
void desymmetrize_FFT3D(algebra::Matrix3D<T> &in) {
  in.resize(in.get_number_of_slices(),
            in.get_number_of_rows(),
            get_fast_fft_size(in.get_number_of_columns()));
}




class FFT1D_slow {
public:
  FFT1D_slow() {prepared_=false;}

  FFT1D_slow(std::vector<double> &in,std::vector<std::complex<double> > &out) {
    prepare(in,out);
  }
  ~FFT1D_slow() { fftw_destroy_plan(FFT_); }

  inline void prepare(std::vector<double> &in,
        std::vector<std::complex<double> > &out) {

    unsigned int in_size=in.size();
    std::vector<std::complex<double> > complex_in(in_size);
    for (unsigned int i=0;i<in_size;++i) {
      std::complex<double> c(in[i],0);
      complex_in[i] = c;
    }

    // Resize for fftw3
    out.resize(in_size);
    // Prepare plan
    FFT_ = fftw_plan_dft_1d(in_size,
        (fftw_complex *)(&complex_in.front()),
        (fftw_complex *)(&out.front()),
        FFTW_FORWARD,
        FFTW_ESTIMATE | FFTW_PRESERVE_INPUT);
    prepared_=true;
  }

  inline void execute() {
    if(!prepared_) {
      IMP_THROW("Attempt to execute a FFT without preparing it before",
                                                            IOException);
    }
    fftw_execute(FFT_);
  }

private:
  bool prepared_;
  fftw_plan FFT_;
};




class IFFT1D_slow {
public:
  IFFT1D_slow() {prepared_=false;}

  IFFT1D_slow(std::vector<std::complex<double> > &in,
              std::vector<double> &out) {
    prepare(in,out);
  }

  ~IFFT1D_slow() { fftw_destroy_plan(IFFT_); }

  inline void prepare(std::vector<std::complex<double> > &in,
               std::vector<double> &out) {
    size_ = in.size();
    out_ = &out; // store a pointer to the output
    complex_out_.resize(size_);
    IFFT_ = fftw_plan_dft_1d(size_,
          (fftw_complex *)(&in.front()),
          (fftw_complex *)(&complex_out_.front()),
          FFTW_BACKWARD,
          FFTW_ESTIMATE);
    prepared_=true;
  }

  inline void execute() {
    if(!prepared_) {
      IMP_THROW("Attempt to execute a IFFT without preparing it before",
                                                            IOException);
    }
    fftw_execute(IFFT_);
    // transfer the output
    (*out_).resize(size_);
    for (unsigned int i=0;i<size_;++i) {
      (*out_)[i] = complex_out_[i].real();
    }
  }

private:
  std::vector<std::complex<double> > complex_out_;
  std::vector<double> *out_;
  bool prepared_;
  unsigned int size_;
  fftw_plan IFFT_;
};



IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_FF_TINTERFACE_H */
