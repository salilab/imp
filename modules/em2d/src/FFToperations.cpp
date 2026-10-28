/**
 *  \file FFToperations.cpp
 *  \brief operations involving FFT
 Copyright 2007-2026 IMP Inventors. All rights reserved.
*/

#include "IMP/em2d/FFToperations.h"
#include "IMP/log.h"
#include "IMP/Pointer.h"
#include <fftw3.h>
#include <algorithm>
#include <complex>
#include <map>
#include <mutex>
#include <new>
#include <utility>
#include <vector>

IMPEM2D_BEGIN_NAMESPACE

namespace {

typedef std::complex<double> Complex;

/* Allocates with fftw_malloc, so that every array passed to FFTW has the
   same (SIMD) alignment as the arrays the cached plans were created with,
   as fftw_execute_dft_r2c() and fftw_execute_dft_c2r() require. */
template <class T>
struct FFTWAllocator {
  typedef T value_type;
  FFTWAllocator() {}
  template <class U>
  FFTWAllocator(const FFTWAllocator<U> &) {}
  T *allocate(std::size_t n) {
    void *p = fftw_malloc(n * sizeof(T));
    if (!p) throw std::bad_alloc();
    return static_cast<T *>(p);
  }
  void deallocate(T *p, std::size_t) { fftw_free(p); }
};

template <class T, class U>
bool operator==(const FFTWAllocator<T> &, const FFTWAllocator<U> &) {
  return true;
}

template <class T, class U>
bool operator!=(const FFTWAllocator<T> &, const FFTWAllocator<U> &) {
  return false;
}

typedef std::vector<double, FFTWAllocator<double> > RealArray;
typedef std::vector<Complex, FFTWAllocator<Complex> > ComplexArray;

/* FFTW plans for 2D real transforms, created once for each size and then
   reused. Creating a plan is expensive and not thread-safe, so it is
   serialized here; executing a plan on new arrays is thread-safe. */
class PlanCache {
  typedef std::pair<int, int> Size;
  std::map<Size, fftw_plan> forward_, inverse_;
  std::mutex mutex_;

  static void destroy(std::map<Size, fftw_plan> &plans) {
    for (auto &p : plans) fftw_destroy_plan(p.second);
  }

 public:
  ~PlanCache() {
    destroy(forward_);
    destroy(inverse_);
  }

  /* Plans are made on scratch arrays allocated like the ones they will be
     executed on. FFTW_ESTIMATE is used as it is fast and deterministic. */
  fftw_plan get_forward(int rows, int cols) {
    std::lock_guard<std::mutex> lock(mutex_);
    fftw_plan &plan = forward_[Size(rows, cols)];
    if (!plan) {
      RealArray in(rows * cols);
      ComplexArray out(rows * (cols / 2 + 1));
      plan = fftw_plan_dft_r2c_2d(rows, cols, &in[0],
                                  reinterpret_cast<fftw_complex *>(&out[0]),
                                  FFTW_ESTIMATE);
      IMP_INTERNAL_CHECK(plan, "FFTW could not create a plan");
    }
    return plan;
  }

  fftw_plan get_inverse(int rows, int cols) {
    std::lock_guard<std::mutex> lock(mutex_);
    fftw_plan &plan = inverse_[Size(rows, cols)];
    if (!plan) {
      ComplexArray in(rows * (cols / 2 + 1));
      RealArray out(rows * cols);
      plan = fftw_plan_dft_c2r_2d(rows, cols,
                                  reinterpret_cast<fftw_complex *>(&in[0]),
                                  &out[0], FFTW_ESTIMATE);
      IMP_INTERNAL_CHECK(plan, "FFTW could not create a plan");
    }
    return plan;
  }
};

PlanCache &get_plan_cache() {
  static PlanCache cache;
  return cache;
}

/* Non-redundant half of the 2D DFT of a real rows x cols matrix, as used by
   FFTW: rows x (cols/2 + 1) complex values, stored row-major. */
struct HalfSpectrum {
  int rows, cols, half_cols;
  ComplexArray data;
  HalfSpectrum(int r, int c)
      : rows(r), cols(c), half_cols(c / 2 + 1), data(r * (c / 2 + 1)) {}
  Complex &operator()(int k, int l) { return data[k * half_cols + l]; }
  const Complex &operator()(int k, int l) const {
    return data[k * half_cols + l];
  }
};

void check_is_double(const cv::Mat &m, const char *function) {
  IMP_USAGE_CHECK(m.type() == CV_64FC1,
                  "em2d:" << function << ": Matrix must be of type CV_64FC1");
}

// DFT of m, zero-padded to rows x cols
HalfSpectrum get_forward_dft(const cv::Mat &m, int rows, int cols) {
  RealArray in(rows * cols, 0.);
  for (int i = 0; i < m.rows; ++i) {
    for (int j = 0; j < m.cols; ++j) {
      in[i * cols + j] = m.at<double>(i, j);
    }
  }
  HalfSpectrum H(rows, cols);
  fftw_execute_dft_r2c(get_plan_cache().get_forward(rows, cols), &in[0],
                       reinterpret_cast<fftw_complex *>(&H.data[0]));
  return H;
}

/* Inverse DFT of H, scaled by 1/(rows*cols). Stores the top-left
   result.rows x result.cols values in result, which must be allocated.
   H is used as scratch space. */
void get_inverse_dft(HalfSpectrum &H, cv::Mat &result) {
  RealArray out(H.rows * H.cols);
  fftw_execute_dft_c2r(get_plan_cache().get_inverse(H.rows, H.cols),
                       reinterpret_cast<fftw_complex *>(&H.data[0]), &out[0]);
  double scale = 1. / (static_cast<double>(H.rows) * H.cols);
  for (int i = 0; i < result.rows; ++i) {
    for (int j = 0; j < result.cols; ++j) {
      result.at<double>(i, j) = out[i * H.cols + j] * scale;
    }
  }
}

// A := A * conj(B), the equivalent of cv::mulSpectrums(A, B, A, 0, true)
void do_multiply_by_conjugate(HalfSpectrum &A, const HalfSpectrum &B) {
  for (unsigned int i = 0; i < A.data.size(); ++i) {
    A.data[i] *= std::conj(B.data[i]);
  }
}

/* OpenCV's packed (CCS) format for the DFT of a real rows x cols matrix
   stores a rows x cols real matrix. Columns 0 and (if cols is even) cols-1
   hold the DFT columns l=0 and l=cols/2, which are conjugate-symmetric in
   k, so only half of each is stored: Re(0), Re(1), Im(1), Re(2), Im(2), ...
   and Re(rows/2) last if rows is even. The other columns hold Re, Im pairs
   of DFT columns l = 1 .. (cols-1)/2 for every row. */
void do_pack_column(const HalfSpectrum &H, int l, cv::Mat &ccs, int col) {
  ccs.at<double>(0, col) = H(0, l).real();
  for (int k = 1; k <= (H.rows - 1) / 2; ++k) {
    ccs.at<double>(2 * k - 1, col) = H(k, l).real();
    ccs.at<double>(2 * k, col) = H(k, l).imag();
  }
  if (H.rows % 2 == 0) {
    ccs.at<double>(H.rows - 1, col) = H(H.rows / 2, l).real();
  }
}

void do_unpack_column(const cv::Mat &ccs, int col, HalfSpectrum &H, int l) {
  H(0, l) = Complex(ccs.at<double>(0, col), 0.);
  for (int k = 1; k <= (H.rows - 1) / 2; ++k) {
    Complex c(ccs.at<double>(2 * k - 1, col), ccs.at<double>(2 * k, col));
    H(k, l) = c;
    H(H.rows - k, l) = std::conj(c);
  }
  if (H.rows % 2 == 0) {
    H(H.rows / 2, l) = Complex(ccs.at<double>(H.rows - 1, col), 0.);
  }
}

// Stores the top-left ccs.rows x ccs.cols values of H in CCS format
void do_pack_ccs(const HalfSpectrum &H, cv::Mat &ccs) {
  cv::Mat full(H.rows, H.cols, CV_64FC1);
  do_pack_column(H, 0, full, 0);
  for (int k = 0; k < H.rows; ++k) {
    for (int l = 1; l <= (H.cols - 1) / 2; ++l) {
      full.at<double>(k, 2 * l - 1) = H(k, l).real();
      full.at<double>(k, 2 * l) = H(k, l).imag();
    }
  }
  if (H.cols % 2 == 0) {
    do_pack_column(H, H.cols / 2, full, H.cols - 1);
  }
  full(cv::Rect(0, 0, ccs.cols, ccs.rows)).copyTo(ccs);
}

HalfSpectrum get_unpacked_ccs(const cv::Mat &ccs) {
  HalfSpectrum H(ccs.rows, ccs.cols);
  do_unpack_column(ccs, 0, H, 0);
  for (int k = 0; k < H.rows; ++k) {
    for (int l = 1; l <= (H.cols - 1) / 2; ++l) {
      H(k, l) = Complex(ccs.at<double>(k, 2 * l - 1),
                        ccs.at<double>(k, 2 * l));
    }
  }
  if (H.cols % 2 == 0) {
    do_unpack_column(ccs, H.cols - 1, H, H.cols / 2);
  }
  return H;
}

bool get_is_5_smooth(int n) {
  const int factors[] = {2, 3, 5};
  for (int f : factors) {
    while (n % f == 0) n /= f;
  }
  return n == 1;
}

}  // namespace

int get_optimal_dft_size(int n) {
  IMP_USAGE_CHECK(n >= 0, "em2d:get_optimal_dft_size: Negative size");
  int size = std::max(n, 1);
  while (!get_is_5_smooth(size)) ++size;
  return size;
}

void get_autocorrelation2d_no_preprocessing(const cv::Mat &M, cv::Mat &corr) {
  IMP_LOG_VERBOSE("Computing 2D autocorrelation no preprocessing" << std::endl);
  IMP_USAGE_CHECK(((M.rows != 0) && (M.cols != 0)),
                  "em2d:get_autocorrelation2d: Output matrix is empty");
  check_is_double(M, "get_autocorrelation2d_no_preprocessing");
  HalfSpectrum H = get_unpacked_ccs(M);
  do_multiply_by_conjugate(H, H);
  get_inverse_dft(H, corr);
  do_matrix_to_image_flip(corr);
}

void get_autocorrelation2d(const cv::Mat &m, cv::Mat &corr) {
  IMP_LOG_VERBOSE("Computing 2D autocorrelation " << std::endl);
  check_is_double(m, "get_autocorrelation2d");
  // pad to the optimal size for a faster DFT
  HalfSpectrum H = get_forward_dft(m, get_optimal_dft_size(m.rows),
                                   get_optimal_dft_size(m.cols));
  do_multiply_by_conjugate(H, H);
  corr.create(m.rows, m.cols, m.type());
  get_inverse_dft(H, corr);
  em2d::do_matrix_to_image_flip(corr);
}

void get_correlation2d(const cv::Mat &A, const cv::Mat &B, cv::Mat &corr) {

  IMP_LOG_VERBOSE("Computing 2D correlation " << std::endl);

  IMP_USAGE_CHECK(((A.rows == B.rows) && (A.cols == B.cols)),
                  "em2d:get_correlation2d: Matrices have different size.");
  check_is_double(A, "get_correlation2d");
  check_is_double(B, "get_correlation2d");
  // pad to the optimal size for a faster DFT
  int rows = get_optimal_dft_size(A.rows);
  int cols = get_optimal_dft_size(A.cols);
  HalfSpectrum HA = get_forward_dft(A, rows, cols);
  HalfSpectrum HB = get_forward_dft(B, rows, cols);
  do_multiply_by_conjugate(HA, HB);
  corr.create(A.rows, A.cols, A.type());
  get_inverse_dft(HA, corr);
  do_matrix_to_image_flip(corr);
}

void get_correlation2d_no_preprocessing(const cv::Mat &M1, const cv::Mat &M2,
                                        cv::Mat &corr) {

  IMP_LOG_VERBOSE("Computing 2D correlation no preprocessing " << std::endl);

  IMP_USAGE_CHECK(((M1.rows == M2.rows) && (M1.cols == M2.cols)),
                  "em2d:get_correlation2d: Matrices have different size.");
  check_is_double(M1, "get_correlation2d_no_preprocessing");
  check_is_double(M2, "get_correlation2d_no_preprocessing");
  HalfSpectrum H1 = get_unpacked_ccs(M1);
  HalfSpectrum H2 = get_unpacked_ccs(M2);
  do_multiply_by_conjugate(H1, H2);
  get_inverse_dft(H1, corr);
  do_matrix_to_image_flip(corr);
}

void get_fft_using_optimal_size(const cv::Mat &m, cv::Mat &M) {
  check_is_double(m, "get_fft_using_optimal_size");
  HalfSpectrum H = get_forward_dft(m, get_optimal_dft_size(m.rows),
                                   get_optimal_dft_size(m.cols));
  // Keep only the top-left region of the packed spectrum, with the size of m
  M.create(m.rows, m.cols, CV_64FC1);
  do_pack_ccs(H, M);
}

void get_spectrum(const cv::Mat &m, cv::Mat &real, cv::Mat &imag) {
  check_is_double(m, "get_spectrum");
  // Shift to get a centered spectrum
  cv::Mat shifted(m.rows, m.cols, CV_64FC1);
  for (int i = 0; i < m.rows; ++i) {
    for (int j = 0; j < m.cols; ++j) {
      if ((i + j) % 2 == 1) {  // the sum is an odd number
        shifted.at<double>(i, j) = (-1) * m.at<double>(i, j);
      } else {
        shifted.at<double>(i, j) = m.at<double>(i, j);
      }
    }
  }
  int rows = get_optimal_dft_size(m.rows);
  int cols = get_optimal_dft_size(m.cols);
  HalfSpectrum H = get_forward_dft(shifted, rows, cols);
  // Recover the full spectrum using that it is conjugate-symmetric
  real.create(m.rows, m.cols, m.type());
  imag.create(m.rows, m.cols, m.type());
  for (int k = 0; k < m.rows; ++k) {
    for (int l = 0; l < m.cols; ++l) {
      Complex c = l < H.half_cols
                      ? H(k, l)
                      : std::conj(H((rows - k) % rows, cols - l));
      real.at<double>(k, l) = c.real();
      imag.at<double>(k, l) = c.imag();
    }
  }
}

void do_matrix_to_image_flip(cv::Mat &m) {
  int half_rows = m.rows / 2;
  int half_columns = m.cols / 2;
  int new_i, new_j;
  for (int i = 0; i < m.rows; ++i) {
    for (int j = 0; j < half_columns; ++j) {
      if (i >= half_rows) {
        new_i = i - half_rows;
      } else {
        new_i = i + half_rows;
      }
      new_j = j + half_columns;
      std::swap(m.at<double>(i, j), m.at<double>(new_i, new_j));
    }
  }
}

IMPEM2D_END_NAMESPACE
