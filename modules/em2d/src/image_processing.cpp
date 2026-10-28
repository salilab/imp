/**
 *  \file image_processing.cpp
 *  \brief image processing for EM
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
*/

#include "IMP/em2d/image_processing.h"
#include "IMP/em2d/internal/CenteredMat.h"
#include "IMP/em2d/internal/image_processing_helper.h"
#include "IMP/em2d/Image.h"
#include "IMP/em2d/SpiderImageReaderWriter.h"
#include "IMP/algebra/eigen_analysis.h"
#include "IMP/check_macros.h"
#include "IMP/exception.h"
#include "IMP/macros.h"
#include "IMP/base_types.h"
#include "IMP/random.h"
#include "boost/random.hpp"
#include "boost/version.hpp"
#include <algorithm>
#include <limits>
#include <list>
#include <queue>
#include <cmath>
#include <utility>
#include <vector>

IMPEM2D_BEGIN_NAMESPACE

namespace {

typedef std::vector<std::pair<int, int> > Offsets;

/* Offsets (row, column) of the structuring element (the nonzero entries of
   kernel) relative to its anchor, the kernel center. An empty kernel is a
   3x3 square, as for OpenCV's dilate() and erode(). */
Offsets get_element_offsets(const DoubleMatrix &kernel) {
  Offsets offsets;
  if (kernel.size() == 0) {
    for (int i = -1; i <= 1; ++i) {
      for (int j = -1; j <= 1; ++j) offsets.push_back(std::make_pair(i, j));
    }
    return offsets;
  }
  int anchor_row = kernel.rows() / 2, anchor_col = kernel.cols() / 2;
  for (int i = 0; i < kernel.rows(); ++i) {
    for (int j = 0; j < kernel.cols(); ++j) {
      if (kernel(i, j) != 0) {
        offsets.push_back(std::make_pair(i - anchor_row, j - anchor_col));
      }
    }
  }
  return offsets;
}

/* Grayscale dilation (maximum over the structuring element) or erosion
   (minimum), as OpenCV's dilate() and erode() with their default arguments.
   Pixels outside the image are ignored. NaN values are not supported.
   result can be m. */
template <class T>
void do_morphology(const internal::RowMajorArray<T> &m,
                   internal::RowMajorArray<T> &result,
                   const DoubleMatrix &kernel, bool dilate) {
  Offsets offsets = get_element_offsets(kernel);
  internal::RowMajorArray<T> out(m.rows(), m.cols());
  for (int i = 0; i < m.rows(); ++i) {
    for (int j = 0; j < m.cols(); ++j) {
      T value = dilate ? std::numeric_limits<T>::lowest()
                       : std::numeric_limits<T>::max();
      for (const auto &o : offsets) {
        int r = i + o.first, c = j + o.second;
        if (r < 0 || r >= m.rows() || c < 0 || c >= m.cols()) continue;
        T v = m(r, c);
        if (dilate ? v > value : v < value) value = v;
      }
      out(i, j) = value;
    }
  }
  result = out;
}

template <class T>
void do_dilate(const internal::RowMajorArray<T> &m,
               internal::RowMajorArray<T> &result, const DoubleMatrix &kernel) {
  do_morphology(m, result, kernel, true);
}

template <class T>
void do_erode(const internal::RowMajorArray<T> &m,
              internal::RowMajorArray<T> &result, const DoubleMatrix &kernel) {
  do_morphology(m, result, kernel, false);
}

/* Index into a sequence of length n, reflecting at the ends without
   repeating the end values (as OpenCV's BORDER_REFLECT_101) */
int get_reflected(int p, int n) {
  if (n == 1) return 0;
  while (p < 0 || p >= n) {
    if (p < 0) p = -p;
    if (p >= n) p = 2 * n - p - 2;
  }
  return p;
}

/* Mean over a ksize x ksize window around each pixel of a matrix, with
   reflected borders. The sums are done in the same order as OpenCV's
   boxFilter(), to give the same results. */
void apply_box_filter(const DoubleMatrix &m, DoubleMatrix &result, int ksize) {
  IMP_USAGE_CHECK(ksize > 0, "em2d: Kernel size must be positive");
  int rows = m.rows(), cols = m.cols(), anchor = ksize / 2;
  // Horizontal sums, for every row of the vertically extended matrix
  int ext_rows = rows + ksize - 1;
  std::vector<std::vector<double> > row_sums(ext_rows,
                                             std::vector<double>(cols));
  std::vector<double> ext_row(cols + ksize - 1);
  for (int er = 0; er < ext_rows; ++er) {
    int r = get_reflected(er - anchor, rows);
    for (int j = 0; j < cols + ksize - 1; ++j) {
      ext_row[j] = m(r, get_reflected(j - anchor, cols));
    }
    std::vector<double> &d = row_sums[er];
    if (ksize <= 5) {
      for (int j = 0; j < cols; ++j) {
        double s = ext_row[j];
        for (int k = 1; k < ksize; ++k) s += ext_row[j + k];
        d[j] = s;
      }
    } else {
      // running sum
      double s = 0.;
      for (int j = 0; j < ksize; ++j) s += ext_row[j];
      d[0] = s;
      for (int j = 0; j + 1 < cols; ++j) {
        s += ext_row[j + ksize] - ext_row[j];
        d[j + 1] = s;
      }
    }
  }
  // Vertical running sums
  double scale = 1. / (ksize * ksize);
  DoubleMatrix out(rows, cols);
  std::vector<double> sum(cols, 0.);
  for (int er = 0; er < ksize - 1; ++er) {
    for (int j = 0; j < cols; ++j) sum[j] += row_sums[er][j];
  }
  for (int i = 0; i < rows; ++i) {
    const std::vector<double> &added = row_sums[i + ksize - 1];
    const std::vector<double> &removed = row_sums[i];
    for (int j = 0; j < cols; ++j) {
      double s = sum[j] + added[j];
      out(i, j) = scale != 1 ? s * scale : s;
      sum[j] = s - removed[j];
    }
  }
  result = out;
}

/* Set the pixels of a filled circle centered at column x and row y to
   value, giving the same pixels as OpenCV's circle(m, cv::Point(x, y),
   radius, value, -1) */
void do_draw_filled_circle(IntMatrix &m, int x, int y, int radius,
                           int value) {
  auto fill_row = [&m, value](int row, int col1, int col2) {
    if (row < 0 || row >= m.rows()) return;
    for (int c = std::max(col1, 0); c <= std::min(col2, int(m.cols()) - 1);
         ++c) {
      m(row, c) = value;
    }
  };
  // Midpoint circle algorithm
  int err = 0, dx = radius, dy = 0, plus = 1, minus = (radius << 1) - 1;
  while (dx >= dy) {
    fill_row(y - dy, x - dx, x + dx);
    fill_row(y + dy, x - dx, x + dx);
    fill_row(y - dx, x - dy, x + dy);
    fill_row(y + dx, x - dy, x + dy);
    dy++;
    err += plus;
    plus += 2;
    int mask = (err <= 0) - 1;
    err -= minus & mask;
    dx += mask;
    minus -= mask & 2;
  }
}

/* Normalized cross-correlation of templ at every position in m, as a
   matrix of floats, as OpenCV's matchTemplate() with TM_CCORR_NORMED */
void get_normalized_correlation(DoubleMatrix m, DoubleMatrix templ,
                                internal::FloatMatrix &result) {
  // As OpenCV, if the template is the larger, swap the roles
  if (m.rows() < templ.rows() || m.cols() < templ.cols()) {
    IMP_USAGE_CHECK(m.rows() <= templ.rows() && m.cols() <= templ.cols(),
                    "em2d: Template must be larger or smaller than the image "
                    "in both dimensions");
    std::swap(m, templ);
  }
  DoubleMatrix corr;
  internal::get_template_correlation(m, templ, corr);
  double templ_sum2 = 0.;
  for (int a = 0; a < templ.rows(); ++a) {
    for (int b = 0; b < templ.cols(); ++b) {
      templ_sum2 += templ(a, b) * templ(a, b);
    }
  }
  double templ_norm = std::sqrt(templ_sum2);
  // Integral image of the squares, for the norm of each window
  DoubleMatrix sq = DoubleMatrix::Zero(m.rows() + 1, m.cols() + 1);
  for (int i = 0; i < m.rows(); ++i) {
    for (int j = 0; j < m.cols(); ++j) {
      double v = m(i, j);
      sq(i + 1, j + 1) = v * v + sq(i, j + 1) + sq(i + 1, j) - sq(i, j);
    }
  }
  int trows = templ.rows(), tcols = templ.cols();
  result.resize(corr.rows(), corr.cols());
  for (int i = 0; i < corr.rows(); ++i) {
    for (int j = 0; j < corr.cols(); ++j) {
      double num = corr(i, j);
      double window_sum2 = sq(i + trows, j + tcols) - sq(i, j + tcols) -
                           sq(i + trows, j) + sq(i, j);
      // Normalize, treating values just out of [-1, 1] (from rounding) and
      // empty windows as OpenCV does
      double t = window_sum2 > 0 ? std::sqrt(window_sum2) * templ_norm : 0.;
      if (std::abs(num) < t) {
        num /= t;
      } else if (std::abs(num) < t * 1.125) {
        num = num > 0 ? 1 : -1;
      } else {
        num = 0;
      }
      result(i, j) = static_cast<float>(num);
    }
  }
}

/* Value of m at a (fractional) row and column, by bilinear interpolation.
   Positions outside the matrix wrap around (as OpenCV's BORDER_WRAP). */
double get_bilinear_sample(const DoubleMatrix &m, double row, double col) {
  long rows = m.rows(), cols = m.cols();
  auto wrap = [](long p, long n) {
    p %= n;
    return p < 0 ? p + n : p;
  };
  double r0 = std::floor(row), c0 = std::floor(col);
  double fr = row - r0, fc = col - c0;
  long i0 = wrap(static_cast<long>(r0), rows);
  long i1 = wrap(static_cast<long>(r0) + 1, rows);
  long j0 = wrap(static_cast<long>(c0), cols);
  long j1 = wrap(static_cast<long>(c0) + 1, cols);
  return (1 - fr) * ((1 - fc) * m(i0, j0) + fc * m(i0, j1)) +
         fr * ((1 - fc) * m(i1, j0) + fc * m(i1, j1));
}

}  // namespace

void do_remove_small_objects(IntMatrix &m, double percentage, int background,
                             int foreground) {
  // Convert the image to integers in order to compute labels
  IntMatrix aux;
  int labels_found = do_labeling(m, aux);  // aux contains the labels

  // Count pixels of each object
  Ints pixel_count(labels_found, 0);
  for (Eigen::Index i = 0; i < aux.size(); ++i) {
    int val = aux.data()[i];
    if (val != background) pixel_count[val - 1]++;
  }
  // Count the pixels of the largest object and the percentage in size of
  // The others
  unsigned int max_pixels =
      *max_element(pixel_count.begin(), pixel_count.end());
  Floats percentages;
  for (Ints::iterator it = pixel_count.begin(); it != pixel_count.end(); ++it) {
    percentages.push_back(double(*it) / double(max_pixels));
  }
  // Remove objects of size lower than the percentage
  for (Eigen::Index i = 0; i < aux.size(); ++i) {
    int &val = aux.data()[i];
    if (val != background) {
      val = percentages[val - 1] > percentage ? foreground : background;
    }
  }
  m = aux;
}

void do_histogram_stretching(DoubleMatrix &m, int boxes, int offset) {

  // Number of possible values for the histogram and maximum value for
  // the stretched image
  double max_val, min_val;
  internal::get_min_and_max(m, &min_val, &max_val);
  double maxmin = max_val - min_val;
  // Histogram of boxes possible values
  Ints hist(boxes, 0);
  for (double *it = m.data(); it != m.data() + m.size(); ++it) {
    double b = static_cast<double>(boxes);
    int k = algebra::get_rounded((b - 1) * ((*it) - min_val) / (maxmin));
    hist[k]++;
  }

  // histogram mode value and index
  double h_mode_val = *std::max_element(hist.begin(), hist.end());
  double h_cut = 0.01 * h_mode_val;  // cut value
  // indexes of the histogram for the cut value
  int i_min = 0, i_max = 0;
  for (unsigned i = 0; i < hist.size(); ++i) {
    if (hist[i] > h_cut) {
      i_min = i;
      break;
    }
  }
  // must be int so it can go negative
  // and, unfortunately, unsigned - unsigned is still unsigned.
  for (int i = static_cast<int>(hist.size()) - 1; i >= 0; --i) {
    if (hist[i] > h_cut) {
      i_max = i;
      break;
    }
  }
  // Allow for some offset
  i_min -= offset;
  if (i_min < 0) {
    i_min = 0;
  }
  i_max += offset;
  if (i_max > (boxes - 1)) {
    i_max = boxes - 1;
  }

  // Min and max values for the new image
  max_val = min_val + (maxmin / (boxes - 1)) * (double(i_max));
  min_val = min_val + (maxmin / (boxes - 1)) * (double(i_min));
  maxmin = max_val - min_val;
  // Stretch
  for (double *it = m.data(); it != m.data() + m.size(); ++it) {
    double val = (*it - min_val) / maxmin;
    if (val < 0) val = 0;
    if (val > 1) val = 1;
    *it = (double(boxes) - 1) * val;
  }
}

void do_dilate_and_shrink_warp(DoubleMatrix &m, const DoubleMatrix &greyscale,
                               DoubleMatrix &kernel) {

  IMP_USAGE_CHECK((m.rows() == greyscale.rows()) &&
                      (m.cols() == greyscale.cols()),
                  "do_dilate_and_shrink_warp: Matrices have different size.");
  int background = 0;
  int foreground = 1;
  DoubleMatrix mask, boundary, temp;
  temp = m;
  unsigned int size_in_pixels, new_size_in_pixels;

  do {
    // pixels above background
    size_in_pixels = 0;
    for (double *it = temp.data(); it != temp.data() + temp.size(); ++it) {
      if (algebra::get_rounded(*it) > background) size_in_pixels++;
    }
    do_dilate(temp, mask, kernel);

    // Compute mean of the grayscale inside the mask and its size
    double mean = 0.0;
    new_size_in_pixels = 0;
    for (int i = 0; i < mask.rows(); ++i) {
      for (int j = 0; j < mask.cols(); ++j) {
        if (algebra::get_rounded(mask(i, j)) > background) {
          new_size_in_pixels++;
          mean += greyscale(i, j);
        }
      }
    }
    mean /= (double)new_size_in_pixels++;
    internal::do_transform(mask, temp, boundary,
                           [](double a, double b) { return a - b; });

    //    xxx.set_data(boundary);
    //    xxx.write("boundary.spi",srw);
    // Erode the mask if pixels in the grayscale are below the mean
    // and are in the boundary
    for (int i = 0; i < greyscale.rows(); ++i) {
      for (int j = 0; j < greyscale.cols(); ++j) {
        internal::Pixel p(i, j);
        if (algebra::get_rounded(mask(p.row, p.col)) == background) {
          temp(p.row, p.col) = background;
        } else if (algebra::get_rounded(boundary(p.row, p.col)) ==
                       foreground &&
                   greyscale(p.row, p.col) < mean) {
          // boundary pixel below the mean, erode
          temp(p.row, p.col) = background;
          new_size_in_pixels--;
        } else {
          temp(p.row, p.col) = foreground;
        }
      }
    }
    /*******/
    //    xxx.set_data(temp);
    //    xxx.write("temp.spi",srw);
    /*******/
  } while (std::abs(static_cast<int>(new_size_in_pixels) -
                    static_cast<int>(size_in_pixels)) > 1);
  // Now temp contains the new mask with size new_size_in_pixels
  m = temp;
}

void apply_threshold(DoubleMatrix &m, DoubleMatrix &result, double threshold) {

  // Remove everything below threshold. Values are rounded to float
  // precision, as before (this used cv::threshold, which does not work
  // with doubles).
  float t = static_cast<float>(threshold);
  internal::do_transform(m, result, [t](double x) {
    float v = static_cast<float>(x);
    return v > t ? static_cast<double>(v) : 0.;
  });
}

void do_combined_fill_holes_and_threshold(DoubleMatrix &m, DoubleMatrix &result,
                                          double n_stddevs, double threshold) {
  do_normalize(m);
  DoubleMatrix temp;
  do_fill_holes(m, temp, n_stddevs);
  do_normalize(temp);
  apply_threshold(temp, result, threshold);
  do_normalize(result);
}

namespace {

template <class T>
internal::RowMajorArray<T> get_transposed(const internal::RowMajorArray<T> &m) {
  return m.transpose();
}

void do_raster_morphologic_reconstruction(const DoubleMatrix &mask,
                                          DoubleMatrix &marker,
                                          int neighbors_mode) {
  // Scan in raster order
  for (int i = 0; i < mask.rows(); ++i) {
    for (int j = 0; j < mask.cols(); ++j) {
      internal::Pixel p(i, j);  // current pixel
      internal::Pixels neighbors =
          internal::get_neighbors2d(p, mask, neighbors_mode, 1);
      Floats marker_values;
      marker_values.push_back(marker(p.row, p.col));
      for (internal::Pixels::iterator it = neighbors.begin();
           it != neighbors.end(); ++it) {
        marker_values.push_back(marker(it->row, it->col));
      }
      double max_val =
          *std::max_element(marker_values.begin(), marker_values.end());
      // Reconstruction
      marker(p.row, p.col) =
          std::min(max_val, mask(p.row, p.col));
    }
  }

  std::queue<internal::Pixel> propagated_pixels;
  // Scan in anti-raster order
  for (int i = mask.rows() - 1; i >= 0; --i) {
    for (int j = mask.cols() - 1; j >= 0; --j) {
      internal::Pixel p(i, j);  // current pixel
      internal::Pixels neighbors =
          internal::get_neighbors2d(p, mask, neighbors_mode, -1);
      // Compute maximum
      double max_val = marker(p.row, p.col);
      for (internal::Pixels::iterator it = neighbors.begin();
           it != neighbors.end(); ++it) {
        max_val = std::max(marker(it->row, it->col), max_val);
        // Check if propagation is required
        if ((marker(it->row, it->col) < marker(p.row, p.col)) &&
            (marker(it->row, it->col) < mask(it->row, it->col))) {
          propagated_pixels.push(p);
        }
      }
      // Reconstruction
      marker(p.row, p.col) =
          std::min(max_val, mask(p.row, p.col));
    }
  }

  // Propagation step
  while (propagated_pixels.empty() == false) {
    internal::Pixel p = propagated_pixels.front();
    propagated_pixels.pop();
    internal::Pixels neighbors = internal::get_neighbors2d(p, mask,
                                                           neighbors_mode, 0);
    // Reconstruction and propagation
    double delta = 0.04;
    for (internal::Pixels::iterator it = neighbors.begin();
         it != neighbors.end(); ++it) {
      if ((marker(it->row, it->col) < marker(p.row, p.col)) &&
          abs(marker(it->row, it->col) -
              mask(it->row, it->col)) > delta) {
        marker(it->row, it->col) = std::min(
            marker(p.row, p.col), mask(it->row, it->col));
        propagated_pixels.push(*it);
      }
    }
  }
}

}  // namespace

void do_morphologic_reconstruction(const DoubleMatrix &mask,
                                   DoubleMatrix &marker,
                                   int neighbors_mode) {

  IMP_USAGE_CHECK(
      (mask.rows() == marker.rows()) && (mask.cols() == marker.cols()),
      "em2d::morphologic_reconstruction: Matrices have different size.");

  /* This reconstruction is approximate (see delta in the propagation step),
     so its result depends on the scan order. The original implementation
     indexed pixels transposed, and so scanned the image column by column.
     To keep the same results, scan the transposed matrices in raster order.
   */
  DoubleMatrix mask_t = get_transposed<double>(mask);
  DoubleMatrix marker_t = get_transposed<double>(marker);
  do_raster_morphologic_reconstruction(mask_t, marker_t, neighbors_mode);
  marker = get_transposed<double>(marker_t);
}

void do_fill_holes(const DoubleMatrix &m, DoubleMatrix &result, double h) {
  double max_m, min_m;
  internal::get_min_and_max(m, &min_m, &max_m);
  double max_plus_h = max_m + h;
  DoubleMatrix mask, marker;
  // this does max_plus_h-m
  internal::do_transform(m, mask,
                         [max_plus_h](double x) { return max_plus_h - x; });
  // The result is the marker. It should be max_plus_m - m - h, but is the same
  // this does max_m-m
  internal::do_transform(m, marker, [max_m](double x) { return max_m - x; });
  do_morphologic_reconstruction(mask, marker, 8);
  // result = max_plus_h - marker
  internal::do_transform(marker, result,
                         [max_plus_h](double x) { return max_plus_h - x; });
}

void get_domes(DoubleMatrix &m, DoubleMatrix &result, double h) {
  DoubleMatrix marker;
  internal::do_transform(m, marker, [h](double x) { return x - h; });
  do_morphologic_reconstruction(m, marker, 8);
  internal::do_transform(m, marker, result,
                         [](double a, double b) { return a - b; });
}

void get_diffusion_filtering_partial_derivative(const DoubleMatrix &m,
                                                DoubleMatrix &der,
                                                double dx, double dy,
                                                double ang) {
  if (m.rows() != der.rows() || m.cols() != der.cols())
    IMP_THROW("Diffusion filter: Derivatives matrix has incorrect size",
              ValueException);

  double c = cos(ang);
  double s = sin(ang);
  double dxdx = dx * dx;
  double dydy = dy * dy;
  double dxdy = dx * dy;
  double Ix, Iy, Ixx, Iyy, Ixy, h;
  const DoubleMatrix &M = m;  // for convenience with the notation
  DoubleMatrix &Der = der;  // same here
  // Value at a pixel, which holds (row, column)
  auto at = [&M](const internal::Pixel &q) { return M(q.row, q.col); };
  for (int i = 0; i < M.rows(); ++i) {
    for (int j = 0; j < M.cols(); ++j) {
      internal::Pixel p(i, j);
      internal::Pixels ns = internal::get_neighbors2d(p, m, 8, 0, true);
      // partial derivatives of m using finite differences
      Ix = (at(ns[2]) - at(p)) / dx;
      Iy = (at(ns[4]) - at(p)) / dy;
      h = 1 / (1 + Ix * Ix + Iy * Iy);  // edge indicator function (h)
      // Second derivatives with finite differences
      Ixx = (at(ns[2]) + at(ns[6]) - 2 * at(p)) / dxdx;
      Iyy = (at(ns[4]) + at(ns[0]) - 2 * at(p)) / dydy;
      Ixy = (at(ns[3]) + at(ns[7]) - at(ns[1]) - at(ns[5])) / (4 * dxdy);
      Der(p.row, p.col) = s * h * (Ixx + Iyy) +
                      c * (-2) * h * h *
                          (Ix * Ix * Ixx + Iy * Iy * Iyy + 2 * Ix * Iy * Ixy);
    }
  }
}

void apply_diffusion_filter(const DoubleMatrix &m, DoubleMatrix &result,
                            double beta,
                            double pixelsize, unsigned int time_steps) {
  if (beta < 0 || beta > 90)
    IMP_THROW("Diffusion filter: Beta not in interval [0,90]", ValueException);

  result = m;
  double dx = pixelsize;
  double dy = pixelsize;
  double dt = 0.5 * (1 / (dx * dx) + 1 / (dy * dy));
  double angle = beta * PI / 180.0;
  // Integrate over time
  DoubleMatrix derivative_time(m.rows(), m.cols());
  for (unsigned int t = 0; t < time_steps; ++t) {
    get_diffusion_filtering_partial_derivative(result, derivative_time, dx, dy,
                                               angle);
    internal::do_transform(result, derivative_time, result,
                           [dt](double r, double d) { return r + d * dt; });
  }
}

void do_segmentation(const DoubleMatrix &m, DoubleMatrix &result,
                     const SegmentationParameters &params) {
  IMP_LOG_VERBOSE("Segmenting image" << std::endl);
  DoubleMatrix temp1, temp2;
  internal::FloatMatrix aux, aux2;
  temp1 = m;
  do_normalize(temp1);
  apply_diffusion_filter(temp1,  // input
                         temp2,  // filtered
                         params.diffusion_beta, params.image_pixel_size,
                         params.diffusion_timesteps);

  do_combined_fill_holes_and_threshold(temp2,  // input
                                       temp1,  // result with holes filled
                                       params.fill_holes_stddevs,
                                       params.threshold);
  // opening (erosion followed by dilation), in floats as before
  aux = temp1.cast<float>();
  do_erode(aux, aux2, params.opening_kernel);
  do_dilate(aux2, aux, params.opening_kernel);
  // threshold the opened image to binary ints, to remove small objects
  IntMatrix Aux(aux.rows(), aux.cols());
  for (int i = 0; i < aux.rows(); ++i) {
    for (int j = 0; j < aux.cols(); ++j) {
      Aux(i, j) = aux(i, j) > 0.f ? params.binary_foreground : 0;
    }
  }
  do_remove_small_objects(Aux, params.remove_sizing_percentage,
                          params.binary_background, params.binary_foreground);
  apply_mask(m, result, Aux, 0.0);  // Aux is the mask
}

int do_labeling(const IntMatrix &m, IntMatrix &mat_to_label) {
  /* The labeling depends on the scan order. The original implementation
     indexed pixels transposed, and so scanned the image column by column.
     To keep the same results, label the transposed matrix in raster order.
   */
  IntMatrix m_t = get_transposed<int>(m), labels_t(m.cols(), m.rows());
  Ints union_find_tree;
  internal::do_binary_matrix_scan(m_t, labels_t, union_find_tree);
  internal::do_flatten_labels(union_find_tree);
  internal::set_final_labels(labels_t, union_find_tree);
  mat_to_label = get_transposed<int>(labels_t);
  int labels = *max_element(union_find_tree.begin(), union_find_tree.end());
  return labels;
}

void apply_mask(const DoubleMatrix &m, DoubleMatrix &result,
                const IntMatrix &mask,
                double val) {
  if (!(m.rows() == mask.rows() && m.cols() == mask.cols()))
    IMP_THROW("apply_mask: The matrix and the mask have different size.",
              ValueException);
  // result can be m
  result.resize(m.rows(), m.cols());
  for (int i = 0; i < m.rows(); ++i) {
    for (int j = 0; j < m.cols(); ++j) {
      result(i, j) = mask(i, j) == 0 ? val : m(i, j);
    }
  }
}

void apply_circular_mask(const DoubleMatrix &mat, DoubleMatrix &result,
                         int radius,
                         double val) {
  IntMatrix mask = create_circular_mask(mat.rows(), mat.cols(), radius);
  apply_mask(mat, result, mask, val);
}

double get_mean(const DoubleMatrix &mat, const IntMatrix &mask) {
  IMP_USAGE_CHECK(mat.rows() == mask.rows() && mat.cols() == mask.cols(),
                  "get_mean: The matrix and the mask have different size.");
  const DoubleMatrix &M = mat;
  double mean = 0;
  double pixels = 0;
  for (int i = 0; i < mask.rows(); ++i) {
    for (int j = 0; j < mask.cols(); ++j) {
      if (mask(i, j) != 0) {
        pixels += 1;
        mean += M(i, j);
      }
    }
  }
  mean /= pixels;
  IMP_LOG_TERSE("Mean within mask " << mean);

  return mean;
}

IntMatrix create_circular_mask(int rows, int cols, int radius) {
  if (rows < 0) IMP_THROW("Negative number of rows", ValueException);
  if (cols < 0) IMP_THROW("Negative number of columns", ValueException);
  if (radius < 0) IMP_THROW("Negative radius", ValueException);
  IntMatrix mask = IntMatrix::Zero(rows, cols);
  // Note that the circle is centered at column rows/2 and row cols/2
  do_draw_filled_circle(mask, floor(rows / 2.), floor(cols / 2.), radius, 100);
  return mask;
}

Floats get_histogram(const DoubleMatrix &m, int bins) {
  IMP_USAGE_CHECK(bins > 0, "get_histogram: Number of bins must be positive");
  Floats histogram(bins);
  double min, max;
  internal::get_min_and_max(m, &min, &max);
  // Step
  double step = (max - min) / static_cast<double>(bins);
  long n_points = m.rows() * m.cols();
  for (const double *it = m.data(); it != m.data() + m.size(); ++it) {
    // The maximum value is on the upper edge of the last bin, so belongs in
    // it. If all values are the same (step is 0), use the first bin.
    int index = 0;
    if (step > 0) {
      index = std::min(static_cast<int>(floor((*it - min) / step)), bins - 1);
    }
    histogram[index] += 1.f / static_cast<double>(n_points);
  }
  return histogram;
}

void apply_variance_filter(const DoubleMatrix &input, DoubleMatrix &filtered,
                           int kernelsize) {
  // Compute the mean for each value with a filter
  // Get the squared means
  IMP_LOG_VERBOSE("Getting squared means" << std::endl);

  auto square = [](double x) { return x * x; };
  DoubleMatrix means, squared_means;
  apply_box_filter(input, means, kernelsize);
  internal::do_transform(means, squared_means, square);

  // Get the means of squares
  IMP_LOG_VERBOSE("Getting means of the squares" << std::endl);

  DoubleMatrix squares;
  internal::do_transform(input, squares, square);
  // make filtered contain the means of the squares
  apply_box_filter(squares, filtered, kernelsize);
  // Variance
  internal::do_transform(filtered, squared_means, filtered,
                         [](double a, double b) { return a - b; });

  IMP_LOG_VERBOSE("Adjusting variance for numerical instability " << std::endl);
  for (double *it = filtered.data();
       it != filtered.data() + filtered.size(); ++it) {
    if (*it < 0) *it = 0;
  }
}

IMP_GCC_PUSH_POP(diagnostic push)
IMP_GCC_PRAGMA(diagnostic ignored "-Wuninitialized")
void add_noise(DoubleMatrix &v, double op1, double op2, const String &mode,
               double /*df*/) {
  IMP_LOG_TERSE("Adding noise: mean " << op1 << " Stddev " << op2 << std::endl);
  // Distribution types
  typedef boost::random::uniform_real_distribution<> unif_distribution;
  typedef boost::normal_distribution<> norm_distribution;
  // Variate generators (put generator and distributions together)
  typedef boost::variate_generator<IMP::RandomNumberGenerator&,
                                   unif_distribution> unif_var_generator;
  typedef boost::variate_generator<IMP::RandomNumberGenerator&,
                                   norm_distribution> norm_var_generator;
  if (mode == "uniform") {
    unif_distribution dist(op1, op2);
    unif_var_generator random_var(IMP::random_number_generator, dist);
    // Add the random numbers
    for (double *it = v.data(); it != v.data() + v.size(); ++it) {
      *it += random_var();
    }
  } else if (mode == "gaussian") {
    norm_distribution dist(op1, op2);
    norm_var_generator random_var(IMP::random_number_generator, dist);
    // Add the random numbers
    for (double *it = v.data(); it != v.data() + v.size(); ++it) {
      *it += random_var();
    }
  } else {
    IMP_THROW("Add_noise: Mode " + mode + " not supported.", ValueException);
  }
}
IMP_GCC_PUSH_POP(diagnostic pop)

void do_resample_polar(const DoubleMatrix &input, DoubleMatrix &resampled,
                       const PolarResamplingParameters &polar_params) {

  internal::FloatMatrix map;
  // If the resampling parameters are not initialized, build a polar map for
  // the resampling.
  if (polar_params.get_is_setup() == false) {
    PolarResamplingParameters p;
    p.setup(input.rows(), input.cols());
    p.set_estimated_number_of_angles(std::min(input.rows(), input.cols()));
    p.create_maps_for_resampling();
    p.get_resampling_map(map);
  } else {
    polar_params.get_resampling_map(map);
  }
  // Sample the input at each point of the map. As before (when this used
  // OpenCV's remap(), which takes (x, y) coordinates), the row coordinates
  // of the map are used as columns, and vice versa.
  DoubleMatrix out(map.rows(), map.cols() / 2);
  for (int i = 0; i < out.rows(); ++i) {
    for (int j = 0; j < out.cols(); ++j) {
      out(i, j) = get_bilinear_sample(input, map(i, 2 * j + 1), map(i, 2 * j));
    }
  }
  resampled = out;
}

void do_normalize(DoubleMatrix &m) {
  double mean, stddev;
  internal::get_mean_and_stddev(m, mean, stddev);
  IMP_LOG_VERBOSE("Matrix of mean: " << mean << " stddev " << stddev
                                     << " normalized. " << std::endl);
  // Write to a new matrix, as before, rather than changing any data that
  // m shares with other matrices. Multiply by 1/stddev rather than dividing,
  // as OpenCV's m / stddev did, to get the same results.
  DoubleMatrix normalized;
  internal::do_transform(m, normalized, [mean, stddev](double x) {
    return (x - mean) * (1. / stddev);
  });
  m = normalized;
}

//! Transform a matrix (the translation is interpreted as cols,rows)
void get_transformed(const DoubleMatrix &input, DoubleMatrix &transformed,
                     const algebra::Transformation2D &T) {
  /* The transformation maps the input to the output with (x, y) coordinates,
     where x is the column and y the row, as with OpenCV's
     getRotationMatrix2D() and warpAffine(), which this used before.
     As before, the center of rotation is (x, y) = (rows/2, cols/2), at float
     precision, and the translation is (x, y). */
  double cx = static_cast<float>(input.rows() / 2.0);
  double cy = static_cast<float>(input.cols() / 2.0);
  double alpha = std::cos(T.get_rotation().get_angle());
  double beta = std::sin(T.get_rotation().get_angle());
  double m00 = alpha, m01 = beta;
  double m02 = (1 - alpha) * cx - beta * cy + T.get_translation()[0];
  double m10 = -beta, m11 = alpha;
  double m12 = beta * cx + (1 - alpha) * cy + T.get_translation()[1];
  // Invert it, to get the input position of each output pixel
  double d = m00 * m11 - m01 * m10;
  d = d != 0 ? 1. / d : 0.;
  double i00 = m11 * d, i01 = -m01 * d, i10 = -m10 * d, i11 = m00 * d;
  double i02 = -i00 * m02 - i01 * m12, i12 = -i10 * m02 - i11 * m12;
  DoubleMatrix out(input.rows(), input.cols());
  for (int y = 0; y < out.rows(); ++y) {
    for (int x = 0; x < out.cols(); ++x) {
      double sx = i00 * x + i01 * y + i02;
      double sy = i10 * x + i11 * y + i12;
      out(y, x) = get_bilinear_sample(input, sy, sx);
    }
  }
  transformed = out;
}

// Using Centered Mat does not allow the first argument to be const.
void do_extend_borders(DoubleMatrix &orig, DoubleMatrix &dst,
                       unsigned int pix) {
  dst = DoubleMatrix::Zero(orig.rows() + 2 * pix, orig.cols() + 2 * pix);
  internal::CenteredMat Orig(orig);
  internal::CenteredMat Dst(dst);
  for (int i = Orig.get_start(0); i <= Orig.get_end(0); ++i) {
    for (int j = Orig.get_start(1); j <= Orig.get_end(1); ++j) {
      Dst(i, j) = Orig(i, j);
    }
  }
}

void do_morphologic_contrast_enhancement(const DoubleMatrix &m,
                                         DoubleMatrix &result,
                                         const DoubleMatrix &kernel,
                                         unsigned int iterations) {
  // Bovik, pg 143

  internal::FloatMatrix dilated, eroded;
  internal::FloatMatrix temp = m.cast<float>();  // in floats, as before

  for (unsigned int i = 0; i < iterations; ++i) {
    do_dilate(temp, dilated, kernel);
    do_erode(temp, eroded, kernel);
    for (int i = 0; i < temp.rows(); ++i) {
      for (int j = 0; j < temp.cols(); ++j) {
        float condition =
            (eroded(i, j) + dilated(i, j)) * 0.5f;
        temp(i, j) = temp(i, j) < condition
                                   ? eroded(i, j)
                                   : dilated(i, j);
      }
    }
  }
  result = temp.cast<double>();
}

void get_morphologic_gradient(const DoubleMatrix &m, DoubleMatrix &result,
                              const DoubleMatrix &kernel) {
  // Bovik, pg 147

  internal::FloatMatrix dilated, eroded;
  internal::FloatMatrix temp = m.cast<float>();  // in floats, as before
  do_dilate(temp, dilated, kernel);
  do_erode(temp, eroded, kernel);
  for (int i = 0; i < temp.rows(); ++i) {
    for (int j = 0; j < temp.cols(); ++j) {
      temp(i, j) = dilated(i, j) - eroded(i, j);
    }
  }
  result = temp.cast<double>();
}

double get_overlap_percentage(DoubleMatrix &m1, DoubleMatrix &m2,
                              const IntPair &center) {

  internal::CenteredMat M1(m1, center.first, center.second);
  internal::CenteredMat M2(m2);
  IMP_USAGE_CHECK(
      (M2.get_start(0) < M1.get_start(0) || M2.get_start(1) < M1.get_start(1) ||
       M2.get_end(0) > M1.get_end(0) || M2.get_end(1) > M1.get_end(1)),
      "em2d::get_overlap_percentage: Second Matrix is not contained in first");

  double pixels_m2 = 0;
  double pixels_overlap = 0;
  for (int i = M2.get_start(0); i < M2.get_end(0); ++i) {
    for (int j = M2.get_start(1); j < M2.get_end(1); ++j) {
      if (M2(i, j) > 0) {
        pixels_m2 += 1;
      } else {
        continue;
      }
      if (M1(i, j) > 0) pixels_overlap += 1;
    }
  }
  return pixels_overlap / pixels_m2;
}

MatchTemplateResults get_best_template_matches(const DoubleMatrix &m,
                                               const DoubleMatrix &templ,
                                               unsigned int n) {
  internal::FloatMatrix result;
  // Round to floats, as before (when this used OpenCV's matchTemplate())
  DoubleMatrix aux_m = m.cast<float>().cast<double>();
  DoubleMatrix aux_templ = templ.cast<float>().cast<double>();
  get_normalized_correlation(aux_m, aux_templ, result);
  std::list<internal::Pixel> locations;
  std::list<float> max_values;
  for (int i = 0; i < result.rows(); ++i) {
    for (int j = 0; j < result.cols(); ++j) {
      internal::Pixel p(i, j);
      float v = result(p.row, p.col);
      if (locations.size() == 0) {
        locations.push_back(p);
        max_values.push_back(v);
      } else {
        std::list<float>::iterator itv = max_values.begin();
        std::list<internal::Pixel>::iterator itp = locations.begin();
        for (; itv != max_values.end(); ++itv, ++itp) {
          bool is_higher = v > *itv ? true : false;
          if (is_higher) {
            locations.insert(itp, p);
            max_values.insert(itv, v);
            if (max_values.size() > n) {
              locations.pop_back();
              max_values.pop_back();
            }
            break;
          }
        }
        if (itv == max_values.end() && (max_values.size() < n)) {
          max_values.push_back(*itv);
          locations.push_back(*itp);
        }
      }
    }
  }

  std::list<internal::Pixel>::const_iterator itp = locations.begin();
  std::list<float>::const_iterator itv = max_values.begin();
  MatchTemplateResults best_locations;
  IMP_LOG_VERBOSE("Best template locations" << std::endl);

  for (; itp != locations.end(); ++itp, ++itv) {
    IMP_LOG_VERBOSE("pixel (" << itp->row << "," << itp->col << ") = " << *itv
                              << std::endl);
    MatchTemplateResult info(IntPair(itp->row, itp->col), *itv);
    best_locations.push_back(info);
  }
  return best_locations;
}

DoubleMatrix crop(const DoubleMatrix &m, const IntPair &center, int size) {
  int s = std::floor(static_cast<double>(size) / 2);
  int x_top_left = center.first - s + 1;
  int y_top_left = center.second - s + 1;

  if (center.first < 0 || center.first > m.rows() || x_top_left < 0 ||
      (x_top_left + s) > m.rows()) {
    IMP_THROW("First coordinate of center is out of bound or size is too big",
              ValueException);
  }
  if (center.second < 0 || center.second > m.cols() || y_top_left < 0 ||
      (y_top_left + s) > m.cols()) {
    IMP_THROW("Second coordinate of center is out of bound or size is too big",
              ValueException);
  }
  // Note that x_top_left is used as a column, and y_top_left as a row
  if (y_top_left + size > m.rows() || x_top_left + size > m.cols()) {
    IMP_THROW("Cropped region is out of bounds", ValueException);
  }
  return m.block(y_top_left, x_top_left, size, size);
}

IMPEM2D_END_NAMESPACE
