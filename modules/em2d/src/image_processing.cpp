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
   3x3 square, as for cv::dilate and cv::erode. */
Offsets get_element_offsets(const cv::Mat &kernel) {
  Offsets offsets;
  if (kernel.empty()) {
    for (int i = -1; i <= 1; ++i) {
      for (int j = -1; j <= 1; ++j) offsets.push_back(std::make_pair(i, j));
    }
    return offsets;
  }
  IMP_USAGE_CHECK(kernel.channels() == 1,
                  "em2d: Kernel must have a single channel");
  int anchor_row = kernel.rows / 2, anchor_col = kernel.cols / 2;
  for (int i = 0; i < kernel.rows; ++i) {
    for (int j = 0; j < kernel.cols; ++j) {
      double v = 0.;
      switch (kernel.depth()) {
        case CV_8U:
          v = kernel.at<unsigned char>(i, j);
          break;
        case CV_32S:
          v = kernel.at<int>(i, j);
          break;
        case CV_32F:
          v = kernel.at<float>(i, j);
          break;
        case CV_64F:
          v = kernel.at<double>(i, j);
          break;
        default:
          IMP_THROW("em2d: Unsupported kernel type", ValueException);
      }
      if (v != 0) {
        offsets.push_back(std::make_pair(i - anchor_row, j - anchor_col));
      }
    }
  }
  return offsets;
}

/* Grayscale dilation (maximum over the structuring element) or erosion
   (minimum), as cv::dilate and cv::erode with their default arguments.
   Pixels outside the image are ignored. NaN values are not supported.
   result can be m. */
template <class T>
void do_morphology(const cv::Mat &m, cv::Mat &result, const cv::Mat &kernel,
                   bool dilate) {
  Offsets offsets = get_element_offsets(kernel);
  cv::Mat out(m.rows, m.cols, m.type());
  for (int i = 0; i < m.rows; ++i) {
    for (int j = 0; j < m.cols; ++j) {
      T value = dilate ? std::numeric_limits<T>::lowest()
                       : std::numeric_limits<T>::max();
      for (const auto &o : offsets) {
        int r = i + o.first, c = j + o.second;
        if (r < 0 || r >= m.rows || c < 0 || c >= m.cols) continue;
        T v = m.at<T>(r, c);
        if (dilate ? v > value : v < value) value = v;
      }
      out.at<T>(i, j) = value;
    }
  }
  out.copyTo(result);
}

void do_morphology(const cv::Mat &m, cv::Mat &result, const cv::Mat &kernel,
                   bool dilate) {
  if (m.type() == CV_64FC1) {
    do_morphology<double>(m, result, kernel, dilate);
  } else {
    IMP_USAGE_CHECK(m.type() == CV_32FC1,
                    "em2d: Matrix must be of type CV_64FC1 or CV_32FC1");
    do_morphology<float>(m, result, kernel, dilate);
  }
}

void do_dilate(const cv::Mat &m, cv::Mat &result, const cv::Mat &kernel) {
  do_morphology(m, result, kernel, true);
}

void do_erode(const cv::Mat &m, cv::Mat &result, const cv::Mat &kernel) {
  do_morphology(m, result, kernel, false);
}

/* Index into a sequence of length n, reflecting at the ends without
   repeating the end values (as cv::BORDER_REFLECT_101) */
int get_reflected(int p, int n) {
  if (n == 1) return 0;
  while (p < 0 || p >= n) {
    if (p < 0) p = -p;
    if (p >= n) p = 2 * n - p - 2;
  }
  return p;
}

/* Mean over a ksize x ksize window around each pixel of a matrix of
   doubles, with reflected borders. The sums are done in the same order as
   cv::boxFilter, to give the same results. */
void apply_box_filter(const cv::Mat &m, cv::Mat &result, int ksize) {
  IMP_USAGE_CHECK(m.type() == CV_64FC1,
                  "em2d: Matrix must be of type CV_64FC1");
  IMP_USAGE_CHECK(ksize > 0, "em2d: Kernel size must be positive");
  int rows = m.rows, cols = m.cols, anchor = ksize / 2;
  // Horizontal sums, for every row of the vertically extended matrix
  int ext_rows = rows + ksize - 1;
  std::vector<std::vector<double> > row_sums(ext_rows,
                                             std::vector<double>(cols));
  std::vector<double> ext_row(cols + ksize - 1);
  for (int er = 0; er < ext_rows; ++er) {
    const double *src = m.ptr<double>(get_reflected(er - anchor, rows));
    for (int j = 0; j < cols + ksize - 1; ++j) {
      ext_row[j] = src[get_reflected(j - anchor, cols)];
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
  cv::Mat out(rows, cols, CV_64FC1);
  std::vector<double> sum(cols, 0.);
  for (int er = 0; er < ksize - 1; ++er) {
    for (int j = 0; j < cols; ++j) sum[j] += row_sums[er][j];
  }
  for (int i = 0; i < rows; ++i) {
    const std::vector<double> &added = row_sums[i + ksize - 1];
    const std::vector<double> &removed = row_sums[i];
    double *d = out.ptr<double>(i);
    for (int j = 0; j < cols; ++j) {
      double s = sum[j] + added[j];
      d[j] = scale != 1 ? s * scale : s;
      sum[j] = s - removed[j];
    }
  }
  out.copyTo(result);
}

/* Set the pixels of a filled circle centered at column x and row y to
   value, giving the same pixels as cv::circle(m, cv::Point(x, y), radius,
   value, -1) */
void do_draw_filled_circle(cv::Mat &m, int x, int y, int radius,
                           double value) {
  auto fill_row = [&m, value](int row, int col1, int col2) {
    if (row < 0 || row >= m.rows) return;
    for (int c = std::max(col1, 0); c <= std::min(col2, m.cols - 1); ++c) {
      m.at<double>(row, c) = value;
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

/* Normalized cross-correlation of templ at every position in m (both
   matrices of doubles), as a matrix of floats, as
   cv::matchTemplate(m, templ, result, cv::TM_CCORR_NORMED) */
void get_normalized_correlation(cv::Mat m, cv::Mat templ, cv::Mat &result) {
  // As OpenCV, if the template is the larger, swap the roles
  if (m.rows < templ.rows || m.cols < templ.cols) {
    IMP_USAGE_CHECK(m.rows <= templ.rows && m.cols <= templ.cols,
                    "em2d: Template must be larger or smaller than the image "
                    "in both dimensions");
    std::swap(m, templ);
  }
  cv::Mat corr;
  internal::get_template_correlation(m, templ, corr);
  double templ_sum2 = 0.;
  for (int a = 0; a < templ.rows; ++a) {
    for (int b = 0; b < templ.cols; ++b) {
      templ_sum2 += templ.at<double>(a, b) * templ.at<double>(a, b);
    }
  }
  double templ_norm = std::sqrt(templ_sum2);
  // Integral image of the squares, for the norm of each window
  cv::Mat sq = cv::Mat::zeros(m.rows + 1, m.cols + 1, CV_64FC1);
  for (int i = 0; i < m.rows; ++i) {
    for (int j = 0; j < m.cols; ++j) {
      double v = m.at<double>(i, j);
      sq.at<double>(i + 1, j + 1) = v * v + sq.at<double>(i, j + 1) +
                                    sq.at<double>(i + 1, j) -
                                    sq.at<double>(i, j);
    }
  }
  result.create(corr.rows, corr.cols, CV_32FC1);
  for (int i = 0; i < corr.rows; ++i) {
    for (int j = 0; j < corr.cols; ++j) {
      double num = corr.at<double>(i, j);
      double window_sum2 = sq.at<double>(i + templ.rows, j + templ.cols) -
                           sq.at<double>(i, j + templ.cols) -
                           sq.at<double>(i + templ.rows, j) +
                           sq.at<double>(i, j);
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
      result.at<float>(i, j) = static_cast<float>(num);
    }
  }
}

}  // namespace

void do_remove_small_objects(cvIntMat &m, double percentage, int background,
                             int foreground) {
  // Convert the image to integers in order to compute labels
  cvIntMat aux;
  int labels_found = do_labeling(m, aux);  // aux contains the labels

  // Count pixels of each object
  Ints pixel_count(labels_found, 0);
  for (cvIntMatIterator it = aux.begin(); it != aux.end(); ++it) {
    int val = (*it);
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
  for (cvIntMatIterator it = aux.begin(); it != aux.end(); ++it) {
    int val = (*it);
    if (val != background) {
      (*it) = percentages[val - 1] > percentage ? foreground : background;
    }
  }
  aux.copyTo(m);
}

void do_histogram_stretching(cv::Mat &m, int boxes, int offset) {

  // Number of possible values for the histogram and maximum value for
  // the stretched image
  double max_val, min_val;
  internal::get_min_and_max(m, &min_val, &max_val);
  double maxmin = max_val - min_val;
  // Histogram of boxes possible values
  Ints hist(boxes, 0);
  for (cvDoubleMatIterator it = m.begin<double>(); it != m.end<double>();
       ++it) {
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
  for (cvDoubleMatIterator it = m.begin<double>(); it != m.end<double>();
       ++it) {
    double val = (*it - min_val) / maxmin;
    if (val < 0) val = 0;
    if (val > 1) val = 1;
    *it = (double(boxes) - 1) * val;
  }
}

void do_dilate_and_shrink_warp(cv::Mat &m, const cv::Mat &greyscale,
                               cv::Mat &kernel) {

  IMP_USAGE_CHECK((m.rows == greyscale.rows) && (m.cols == greyscale.cols),
                  "do_dilate_and_shrink_warp: Matrices have different size.");
  int background = 0;
  int foreground = 1;
  cv::Mat mask, boundary, temp;
  m.copyTo(temp);
  unsigned int size_in_pixels, new_size_in_pixels;

  do {
    // pixels above background
    size_in_pixels = 0;
    for (cvDoubleMatIterator it = temp.begin<double>();
         it != temp.end<double>(); ++it) {
      if (algebra::get_rounded(*it) > background) size_in_pixels++;
    }
    do_dilate(temp, mask, kernel);

    // Compute mean of the grayscale inside the mask and its size
    double mean = 0.0;
    new_size_in_pixels = 0;
    for (int i = 0; i < mask.rows; ++i) {
      for (int j = 0; j < mask.cols; ++j) {
        if (algebra::get_rounded(mask.at<double>(i, j)) > background) {
          new_size_in_pixels++;
          mean += greyscale.at<double>(i, j);
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
    for (int i = 0; i < greyscale.rows; ++i) {
      for (int j = 0; j < greyscale.cols; ++j) {
        cvPixel p(i, j);
        if (algebra::get_rounded(mask.at<double>(p.x, p.y)) == background) {
          temp.at<double>(p.x, p.y) = background;
        } else if (algebra::get_rounded(boundary.at<double>(p.x, p.y)) ==
                       foreground &&
                   greyscale.at<double>(p.x, p.y) < mean) {
          // boundary pixel below the mean, erode
          temp.at<double>(p.x, p.y) = background;
          new_size_in_pixels--;
        } else {
          temp.at<double>(p.x, p.y) = foreground;
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
  temp.copyTo(m);
}

void apply_threshold(cv::Mat &m, cv::Mat &result, double threshold) {

  // Remove everything below threshold. Values are rounded to float
  // precision, as before (this used cv::threshold, which does not work
  // with doubles).
  float t = static_cast<float>(threshold);
  internal::do_transform(m, result, [t](double x) {
    float v = static_cast<float>(x);
    return v > t ? static_cast<double>(v) : 0.;
  });
}

void do_combined_fill_holes_and_threshold(cv::Mat &m, cv::Mat &result,
                                          double n_stddevs, double threshold) {
  do_normalize(m);
  cv::Mat temp, temp2;
  do_fill_holes(m, temp, n_stddevs);
  do_normalize(temp);
  apply_threshold(temp, result, threshold);

  //  temp.convertTo(temp2,CV_32FC1); // threshold does not work with doubles
  //  // Remove everything below threshold
  //  double dummy = 0.0
  //  cv::threshold(temp2,temp,threshold, dummy,cv::THRESH_TOZERO);
  //  temp.convertTo(result, CV_64FC1);
  do_normalize(result);
}

namespace {

template <class T>
cv::Mat_<T> get_transposed(const cv::Mat_<T> &m) {
  cv::Mat_<T> t(m.cols, m.rows);
  for (int i = 0; i < m.rows; ++i) {
    for (int j = 0; j < m.cols; ++j) t(j, i) = m(i, j);
  }
  return t;
}

void do_raster_morphologic_reconstruction(const cv::Mat &mask,
                                          cv::Mat &marker,
                                          int neighbors_mode) {
  // Scan in raster order
  for (int i = 0; i < mask.rows; ++i) {
    for (int j = 0; j < mask.cols; ++j) {
      cvPixel p(i, j);  // current pixel
      cvPixels neighbors =
          internal::get_neighbors2d(p, mask, neighbors_mode, 1);
      Floats marker_values;
      marker_values.push_back(marker.at<double>(p.x, p.y));
      for (cvPixels::iterator it = neighbors.begin(); it != neighbors.end();
           ++it) {
        marker_values.push_back(marker.at<double>(it->x, it->y));
      }
      double max_val =
          *std::max_element(marker_values.begin(), marker_values.end());
      // Reconstruction
      marker.at<double>(p.x, p.y) =
          std::min(max_val, mask.at<double>(p.x, p.y));
    }
  }

  std::queue<cvPixel> propagated_pixels;
  // Scan in anti-raster order
  for (int i = mask.rows - 1; i >= 0; --i) {
    for (int j = mask.cols - 1; j >= 0; --j) {
      cvPixel p(i, j);  // current pixel
      cvPixels neighbors =
          internal::get_neighbors2d(p, mask, neighbors_mode, -1);
      // Compute maximum
      double max_val = marker.at<double>(p.x, p.y);
      for (cvPixels::iterator it = neighbors.begin(); it != neighbors.end();
           ++it) {
        max_val = std::max(marker.at<double>(it->x, it->y), max_val);
        // Check if propagation is required
        if ((marker.at<double>(it->x, it->y) < marker.at<double>(p.x, p.y)) &&
            (marker.at<double>(it->x, it->y) < mask.at<double>(it->x, it->y))) {
          propagated_pixels.push(p);
        }
      }
      // Reconstruction
      marker.at<double>(p.x, p.y) =
          std::min(max_val, mask.at<double>(p.x, p.y));
    }
  }

  // Propagation step
  while (propagated_pixels.empty() == false) {
    cvPixel p = propagated_pixels.front();
    propagated_pixels.pop();
    cvPixels neighbors = internal::get_neighbors2d(p, mask, neighbors_mode, 0);
    // Reconstruction and propagation
    double delta = 0.04;
    for (cvPixels::iterator it = neighbors.begin(); it != neighbors.end();
         ++it) {
      if ((marker.at<double>(it->x, it->y) < marker.at<double>(p.x, p.y)) &&
          abs(marker.at<double>(it->x, it->y) -
              mask.at<double>(it->x, it->y)) > delta) {
        marker.at<double>(it->x, it->y) = std::min(
            marker.at<double>(p.x, p.y), mask.at<double>(it->x, it->y));
        propagated_pixels.push(*it);
      }
    }
  }
}

}  // namespace

void do_morphologic_reconstruction(const cv::Mat &mask, cv::Mat &marker,
                                   int neighbors_mode) {

  IMP_USAGE_CHECK(
      (mask.rows == marker.rows) && (mask.cols == marker.cols),
      "em2d::morphologic_reconstruction: Matrices have different size.");
  IMP_USAGE_CHECK(mask.type() == CV_64FC1 && marker.type() == CV_64FC1,
                  "em2d::morphologic_reconstruction: Matrices must be of "
                  "type CV_64FC1");

  /* This reconstruction is approximate (see delta in the propagation step),
     so its result depends on the scan order. The original implementation
     indexed pixels transposed, and so scanned the image column by column.
     To keep the same results, scan the transposed matrices in raster order.
   */
  cv::Mat mask_t = get_transposed<double>(mask);
  cv::Mat marker_t = get_transposed<double>(marker);
  do_raster_morphologic_reconstruction(mask_t, marker_t, neighbors_mode);
  get_transposed<double>(marker_t).copyTo(marker);
}

void do_fill_holes(const cv::Mat &m, cv::Mat &result, double h) {
  double max_m, min_m;
  internal::get_min_and_max(m, &min_m, &max_m);
  double max_plus_h = max_m + h;
  cv::Mat mask, marker;
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

void get_domes(cv::Mat &m, cv::Mat &result, double h) {
  cv::Mat marker;
  internal::do_transform(m, marker, [h](double x) { return x - h; });
  do_morphologic_reconstruction(m, marker, 8);
  internal::do_transform(m, marker, result,
                         [](double a, double b) { return a - b; });
}

void get_diffusion_filtering_partial_derivative(const cv::Mat &m, cv::Mat &der,
                                                double dx, double dy,
                                                double ang) {
  if (m.rows != der.rows || m.cols != der.cols)
    IMP_THROW("Diffusion filter: Derivatives matrix has incorrect size",
              ValueException);

  double c = cos(ang);
  double s = sin(ang);
  double dxdx = dx * dx;
  double dydy = dy * dy;
  double dxdy = dx * dy;
  double Ix, Iy, Ixx, Iyy, Ixy, h;
  cv::Mat_<double> M = m;  // for convenience with the notation (does not copy)
  cv::Mat_<double> Der = der;  // same here
  // Value at a pixel, which holds (row, column)
  auto at = [&M](const cvPixel &q) { return M(q.x, q.y); };
  for (int i = 0; i < M.rows; ++i) {
    for (int j = 0; j < M.cols; ++j) {
      cvPixel p(i, j);
      cvPixels ns = internal::get_neighbors2d(p, m, 8, 0, true);
      // partial derivatives of m using finite differences
      Ix = (at(ns[2]) - at(p)) / dx;
      Iy = (at(ns[4]) - at(p)) / dy;
      h = 1 / (1 + Ix * Ix + Iy * Iy);  // edge indicator function (h)
      // Second derivatives with finite differences
      Ixx = (at(ns[2]) + at(ns[6]) - 2 * at(p)) / dxdx;
      Iyy = (at(ns[4]) + at(ns[0]) - 2 * at(p)) / dydy;
      Ixy = (at(ns[3]) + at(ns[7]) - at(ns[1]) - at(ns[5])) / (4 * dxdy);
      Der(p.x, p.y) = s * h * (Ixx + Iyy) +
                      c * (-2) * h * h *
                          (Ix * Ix * Ixx + Iy * Iy * Iyy + 2 * Ix * Iy * Ixy);
    }
  }
}

void apply_diffusion_filter(const cv::Mat &m, cv::Mat &result, double beta,
                            double pixelsize, unsigned int time_steps) {
  if (beta < 0 || beta > 90)
    IMP_THROW("Diffusion filter: Beta not in interval [0,90]", ValueException);

  m.copyTo(result);
  double dx = pixelsize;
  double dy = pixelsize;
  double dt = 0.5 * (1 / (dx * dx) + 1 / (dy * dy));
  double angle = beta * PI / 180.0;
  // Integrate over time
  cv::Mat derivative_time(m.rows, m.cols, m.type());
  for (unsigned int t = 0; t < time_steps; ++t) {
    get_diffusion_filtering_partial_derivative(result, derivative_time, dx, dy,
                                               angle);
    internal::do_transform(result, derivative_time, result,
                           [dt](double r, double d) { return r + d * dt; });
  }
}

void do_segmentation(const cv::Mat &m, cv::Mat &result,
                     const SegmentationParameters &params) {
  IMP_LOG_VERBOSE("Segmenting image" << std::endl);
  cv::Mat temp1, temp2;  // to store doubles
  cv::Mat aux, aux2;     // to store floats
  m.copyTo(temp1);
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
  temp1.convertTo(aux, CV_32FC1);
  do_erode(aux, aux2, params.opening_kernel);
  do_dilate(aux2, aux, params.opening_kernel);
  // threshold the opened image to binary ints, to remove small objects
  cvIntMat Aux(aux.rows, aux.cols);
  for (int i = 0; i < aux.rows; ++i) {
    for (int j = 0; j < aux.cols; ++j) {
      Aux(i, j) = aux.at<float>(i, j) > 0.f ? params.binary_foreground : 0;
    }
  }
  do_remove_small_objects(Aux, params.remove_sizing_percentage,
                          params.binary_background, params.binary_foreground);
  apply_mask(m, result, Aux, 0.0);  // Aux is the mask
}

int do_labeling(const cvIntMat &m, cvIntMat &mat_to_label) {
  /* The labeling depends on the scan order. The original implementation
     indexed pixels transposed, and so scanned the image column by column.
     To keep the same results, label the transposed matrix in raster order.
   */
  cvIntMat m_t = get_transposed<int>(m), labels_t(m.cols, m.rows);
  Ints union_find_tree;
  internal::do_binary_matrix_scan(m_t, labels_t, union_find_tree);
  internal::do_flatten_labels(union_find_tree);
  internal::set_final_labels(labels_t, union_find_tree);
  get_transposed<int>(labels_t).copyTo(mat_to_label);
  int labels = *max_element(union_find_tree.begin(), union_find_tree.end());
  return labels;
}

void apply_mask(const cv::Mat &m, cv::Mat &result, const cvIntMat &mask,
                double val) {
  if (!(m.rows == mask.rows && m.cols == mask.cols))
    IMP_THROW("apply_mask: The matrix and the mask have different size.",
              ValueException);
  result.create(m.rows, m.cols, m.type());
  cvDoubleMat M = m;
  cvDoubleMat R = result;
  for (int i = 0; i < M.rows; ++i) {
    for (int j = 0; j < M.cols; ++j) {
      R(i, j) = mask(i, j) == 0 ? val : M(i, j);
    }
  }
}

void apply_circular_mask(const cv::Mat &mat, cv::Mat &result, int radius,
                         double val) {
  cvIntMat mask = create_circular_mask(mat.rows, mat.cols, radius);
  apply_mask(mat, result, mask, val);
}

double get_mean(const cv::Mat &mat, const cvIntMat &mask) {
  IMP_USAGE_CHECK(mat.rows == mask.rows && mat.cols == mask.cols,
                  "get_mean: The matrix and the mask have different size.");
  cvDoubleMat M = mat;
  double mean = 0;
  double pixels = 0;
  for (int i = 0; i < mask.rows; ++i) {
    for (int j = 0; j < mask.cols; ++j) {
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

cvIntMat create_circular_mask(int rows, int cols, int radius) {
  if (rows < 0) IMP_THROW("Negative number of rows", ValueException);
  if (cols < 0) IMP_THROW("Negative number of columns", ValueException);
  if (radius < 0) IMP_THROW("Negative radius", ValueException);
  // cv::Mat mask = cv::Mat::zeros(rows, cols, CV_16UC1);
  cv::Mat mask = cv::Mat::zeros(rows, cols, CV_64FC1);
  // Note that the circle is centered at column rows/2 and row cols/2
  do_draw_filled_circle(mask, floor(rows / 2.), floor(cols / 2.), radius, 100);
  return mask;
}

Floats get_histogram(const cv::Mat &m, int bins) {
  IMP_USAGE_CHECK(bins > 0, "get_histogram: Number of bins must be positive");
  Floats histogram(bins);
  double min, max;
  internal::get_min_and_max(m, &min, &max);
  // Step
  double step = (max - min) / static_cast<double>(bins);
  long n_points = m.rows * m.cols;
  for (cvDoubleConstMatIterator it = m.begin<double>(); it != m.end<double>();
       ++it) {
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

void apply_variance_filter(const cv::Mat &input, cv::Mat &filtered,
                           int kernelsize) {
  // Compute the mean for each value with a filter
  // Get the squared means
  IMP_LOG_VERBOSE("Getting squared means" << std::endl);

  auto square = [](double x) { return x * x; };
  cv::Mat means, squared_means;
  apply_box_filter(input, means, kernelsize);
  internal::do_transform(means, squared_means, square);

  // Get the means of squares
  IMP_LOG_VERBOSE("Getting means of the squares" << std::endl);

  cv::Mat squares;
  internal::do_transform(input, squares, square);
  // make filtered contain the means of the squares
  apply_box_filter(squares, filtered, kernelsize);
  // Variance
  internal::do_transform(filtered, squared_means, filtered,
                         [](double a, double b) { return a - b; });

  IMP_LOG_VERBOSE("Adjusting variance for numerical instability " << std::endl);
  for (cvDoubleMatIterator it = filtered.begin<double>();
       it != filtered.end<double>(); ++it) {
    if (*it < 0) *it = 0;
  }
}

IMP_GCC_PUSH_POP(diagnostic push)
IMP_GCC_PRAGMA(diagnostic ignored "-Wuninitialized")
void add_noise(cv::Mat &v, double op1, double op2, const String &mode,
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
    for (cvDoubleMatIterator it = v.begin<double>(); it != v.end<double>();
         ++it) {
      *it += random_var();
    }
  } else if (mode == "gaussian") {
    norm_distribution dist(op1, op2);
    norm_var_generator random_var(IMP::random_number_generator, dist);
    // Add the random numbers
    for (cvDoubleMatIterator it = v.begin<double>(); it != v.end<double>();
         ++it) {
      *it += random_var();
    }
  } else {
    IMP_THROW("Add_noise: Mode " + mode + " not supported.", ValueException);
  }
}
IMP_GCC_PUSH_POP(diagnostic pop)

void do_resample_polar(const cv::Mat &input, cv::Mat &resampled,
                       const PolarResamplingParameters &polar_params) {

  cv::Mat map_16SC2, map_16UC1;
  // If the resampling parameters are not initialized, build a polar map for
  // the resampling.
  if (polar_params.get_is_setup() == false) {
    PolarResamplingParameters p;
    p.setup(input.rows, input.cols);
    p.set_estimated_number_of_angles(std::min(input.rows, input.cols));
    p.create_maps_for_resampling();
    p.get_resampling_maps(map_16SC2, map_16UC1);
  } else {
    polar_params.get_resampling_maps(map_16SC2, map_16UC1);
  }
  cv::Mat temp, temp2, map2;
  input.convertTo(temp, CV_32FC1);  // remap does not work with doubles, convert
  cv::remap(temp, temp2, map_16SC2, map_16UC1, cv::INTER_LINEAR,
            cv::BORDER_WRAP);
  temp2.convertTo(resampled, CV_64FC1);  // convert back
}

void do_normalize(cv::Mat &m) {
  double mean, stddev;
  internal::get_mean_and_stddev(m, mean, stddev);
  IMP_LOG_VERBOSE("Matrix of mean: " << mean << " stddev " << stddev
                                     << " normalized. " << std::endl);
  // Write to a new matrix, as before, rather than changing any data that
  // m shares with other matrices. Multiply by 1/stddev rather than dividing,
  // as OpenCV's m / stddev did, to get the same results.
  cv::Mat normalized;
  internal::do_transform(m, normalized, [mean, stddev](double x) {
    return (x - mean) * (1. / stddev);
  });
  m = normalized;
}

//! Transform a matrix (the translation is interpreted as cols,rows)
void get_transformed(const cv::Mat &input, cv::Mat &transformed,
                     const algebra::Transformation2D &T) {
  cv::Point2f center(input.rows / 2.0, input.cols / 2.0);
  double angle = 180. * T.get_rotation().get_angle() / PI;
  cv::Mat rot_mat = cv::getRotationMatrix2D(center, angle, 1.0);
  // Careful here. OpenCV convention interprets a translation as col,row
  rot_mat.at<double>(0, 2) += T.get_translation()[0];
  rot_mat.at<double>(1, 2) += T.get_translation()[1];
  cv::Size dsize(input.cols, input.rows);
  // WarpAffine calls remap, that does not work with doubles (CV_64FC1).
  // Use floats CV_32FC1
  cv::Mat temp, temp2;
  input.convertTo(temp, CV_32FC1);
  cv::warpAffine(temp, temp2, rot_mat, dsize, cv::INTER_LINEAR,
                 cv::BORDER_WRAP);
  temp2.convertTo(transformed, CV_64FC1);  // convert back
}

// Using Centered Mat does not allow the first argument to be const.
void do_extend_borders(cv::Mat &orig, cv::Mat &dst, unsigned int pix) {
  dst.create(orig.rows + 2 * pix, orig.cols + 2 * pix, orig.type());
  dst.setTo(0.0);
  internal::CenteredMat Orig(orig);
  internal::CenteredMat Dst(dst);
  for (int i = Orig.get_start(0); i <= Orig.get_end(0); ++i) {
    for (int j = Orig.get_start(1); j <= Orig.get_end(1); ++j) {
      Dst(i, j) = Orig(i, j);
    }
  }
}

void do_morphologic_contrast_enhancement(const cv::Mat &m, cv::Mat &result,
                                         const cv::Mat &kernel,
                                         unsigned int iterations) {
  // Bovik, pg 143

  cv::Mat dilated, eroded;
  cv::Mat temp;
  m.convertTo(temp, CV_32FC1);  // in floats, as before

  for (unsigned int i = 0; i < iterations; ++i) {
    do_dilate(temp, dilated, kernel);
    do_erode(temp, eroded, kernel);
    for (int i = 0; i < temp.rows; ++i) {
      for (int j = 0; j < temp.cols; ++j) {
        float condition =
            (eroded.at<float>(i, j) + dilated.at<float>(i, j)) * 0.5f;
        temp.at<float>(i, j) = temp.at<float>(i, j) < condition
                                   ? eroded.at<float>(i, j)
                                   : dilated.at<float>(i, j);
      }
    }
  }
  temp.convertTo(result, CV_64FC1);
}

void get_morphologic_gradient(const cv::Mat &m, cv::Mat &result,
                              const cv::Mat &kernel) {
  // Bovik, pg 147

  cv::Mat dilated, eroded;
  cv::Mat temp;
  m.convertTo(temp, CV_32FC1);  // in floats, as before
  do_dilate(temp, dilated, kernel);
  do_erode(temp, eroded, kernel);
  for (int i = 0; i < temp.rows; ++i) {
    for (int j = 0; j < temp.cols; ++j) {
      temp.at<float>(i, j) = dilated.at<float>(i, j) - eroded.at<float>(i, j);
    }
  }
  temp.convertTo(result, CV_64FC1);
}

double get_overlap_percentage(cv::Mat &m1, cv::Mat &m2, const IntPair &center) {

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

MatchTemplateResults get_best_template_matches(const cv::Mat &m,
                                               const cv::Mat &templ,
                                               unsigned int n) {
  cv::Mat result, aux_m, aux_templ;
  // Round to floats, as before (when this used cv::matchTemplate)
  m.convertTo(aux_m, CV_32FC1);
  templ.convertTo(aux_templ, CV_32FC1);
  aux_m.convertTo(aux_m, CV_64FC1);
  aux_templ.convertTo(aux_templ, CV_64FC1);
  get_normalized_correlation(aux_m, aux_templ, result);
  /* Write the image with the correlations
  IMP_NEW(Image, img, ());
  cv::Mat temp;
  result.convertTo(temp,CV_64FC1);
  img->set_data(temp);
  IMP_NEW(SpiderImageReaderWriter, srw, ());
  img->write("matches_template.spi", srw);
  */
  std::list<cvPixel> locations;
  std::list<float> max_values;
  for (int i = 0; i < result.rows; ++i) {
    for (int j = 0; j < result.cols; ++j) {
      cvPixel p(i, j);
      float v = result.at<float>(p.x, p.y);
      if (locations.size() == 0) {
        locations.push_back(p);
        max_values.push_back(v);
      } else {
        std::list<float>::iterator itv = max_values.begin();
        std::list<cvPixel>::iterator itp = locations.begin();
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

  std::list<cvPixel>::const_iterator itp = locations.begin();
  std::list<float>::const_iterator itv = max_values.begin();
  MatchTemplateResults best_locations;
  IMP_LOG_VERBOSE("Best template locations" << std::endl);

  // int l = 0;
  for (; itp != locations.end(); ++itp, ++itv) {
    IMP_LOG_VERBOSE("pixel (" << itp->x << "," << itp->y << ") = " << *itv
                              << std::endl);
    MatchTemplateResult info(IntPair(itp->x, itp->y), *itv);
    best_locations.push_back(info);

    /* Write the template on top of the image
    cv::Mat mat;
    m.copyTo(mat);
    cv::Mat roi(mat, cv::Rect(itp->y, itp->x, templ.rows, templ.cols));
    cv::Mat temp = templ * 3;
    roi = roi + temp;
    IMP_NEW(Image, xxx, ());
    xxx->set_data(mat);
    std::ostringstream oss2;
    oss2 << "img-and-template-sol-" << l << ".spi";
    xxx->write(oss2.str(), srw);
    ++l;
    */
  }
  return best_locations;
}

cv::Mat crop(const cv::Mat &m, const IntPair &center, int size) {
  int s = std::floor(static_cast<double>(size) / 2);
  int x_top_left = center.first - s + 1;
  int y_top_left = center.second - s + 1;

  if (center.first < 0 || center.first > m.rows || x_top_left < 0 ||
      (x_top_left + s) > m.rows) {
    IMP_THROW("First coordinate of center is out of bound or size is too big",
              ValueException);
  }
  if (center.second < 0 || center.second > m.cols || y_top_left < 0 ||
      (y_top_left + s) > m.cols) {
    IMP_THROW("Second coordinate of center is out of bound or size is too big",
              ValueException);
  }
  cv::Rect region_of_interest(x_top_left, y_top_left, size, size);
  return m(region_of_interest);
}

IMPEM2D_END_NAMESPACE
