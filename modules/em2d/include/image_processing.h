/**
 *  \file image_processing.h
 *  \brief Image processing functions
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_IMAGE_PROCESSING_H
#define IMPEM2D_IMAGE_PROCESSING_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/em2d/opencv_interface.h"
#include "IMP/em2d/Pixel.h"
#include "IMP/em/Image.h"
#include "IMP/em2d/Image.h"
#include "IMP/em/image_transformations.h"
#include "IMP/algebra/Matrix2D.h"
#include <algorithm>

IMPEM2D_BEGIN_NAMESPACE

//! Adaptive Wiener filter for a 2D matrix m, which is stored in matrix result
/** \param[in] kernel_rows the total number of rows to use in the kernel
               of the filter
    \param[in] kernel_rows the total number of columns to use in the kernel
              of the filter
  \note
**/
 IMPEM2DEXPORT void wiener_filter_2D(algebra::Matrix2D_d &m,
                      algebra::Matrix2D_d &result,
                      const unsigned int kernel_rows=3,
                      const unsigned int kernel_cols=3);



//! Morphological grayscale reconstruction (L Vincent, 1993)
/**
  \param[in] mask image to reconstruct
  \param[out] marker this image contains the initial marker points and will
              contain the final result
  \param[in] neighbors_mode number of neighbors for a pixel to consider when
             doing the morphological reconstruction (values: 4, 8).
**/
 IMPEM2DEXPORT void morphological_reconstruction(algebra::Matrix2D_d &mask,
                      algebra::Matrix2D_d &marker,
                      int neighbors_mode=4);



//! Returns the neighbors for a givel pixel in a given mode
/**
  \ note IMPORTANT: The neighbors are returned in CLOCKWISE order starting
                    with i-1,j+0  (12 o'clock)
  \param[in] p the pixel to get the neighbors for
  \param[in] matrix where the neighbors are computed
  \param[in] mode modes supported for neighbors: 4 (default) and 8 neighbors
  \params[in] sign of the neighbors requested
              0 - all neighbors (default)
              1 - positive or before raster-order neighbors
              -1 - negative or before antiraster-order neighbors
  \params[in] cycle if true, the neighbors are cyclical at the border of the
              matrix. Eg. the neighbor to the right of last column, is the
              initial column. If this parameter is false (default) the
              neighbors outside the matrix are removed.
**/
 IMPEM2DEXPORT Pixels compute_neighbors_2D(
                            const Pixel &p,const algebra::Matrix2D_d &m,
                             int mode=4,
                             int sign=0,
                             bool cycle=false);



//! Fills the holes in the matrix m of height h
 IMPEM2DEXPORT void fill_holes(
            algebra::Matrix2D_d &m, algebra::Matrix2D_d &result,double h);

//! Gets the domes of m with height h
 IMPEM2DEXPORT void get_domes(
                  algebra::Matrix2D_d &m,algebra::Matrix2D_d &result,double h);


 IMPEM2DEXPORT void preprocess_em2d(algebra::Matrix2D_d &m,
                     algebra::Matrix2D_d &result,
                     double n_stddevs);




//! Morphological dilation of a grayscale matrix m.
/**
  \param[in] m Matrix to dilate
  \param[in] kernel structuring element used for the opening
  \param[in] result resulting matrix
**/
 IMPEM2DEXPORT void dilation(const algebra::Matrix2D_d &m,
              algebra::Matrix2D_d &kernel,
              algebra::Matrix2D_d &result);

//! Morphological erosion of a grayscale matrix m.
/**
  \param[in] m Matrix to dilate
  \param[in] kernel structuring element used for the opening
  \param[in] result resulting matrix
**/
IMPEM2DEXPORT void erosion(const algebra::Matrix2D_d &m,
              algebra::Matrix2D_d &kernel,
              algebra::Matrix2D_d &result);
//! Morphological opening of a grayscale matrix m.
/**
  \param[in] m Matrix to dilate
  \param[in] kernel structuring element used for the opening
  \param[in] result resulting matrix
**/
IMPEM2DEXPORT void opening(const algebra::Matrix2D_d &m,
              algebra::Matrix2D_d &kernel,
              algebra::Matrix2D_d &result);

//! Morphological closing of a grayscale matrix m.
/**
  \param[in] m Matrix to dilate
  \param[in] kernel structuring element used for the opening
  \param[in] result resulting matrix
**/
 IMPEM2DEXPORT void closing(const algebra::Matrix2D_d &m,
              algebra::Matrix2D_d &kernel,
              algebra::Matrix2D_d &result);


//! Thresholding to get a binary image
/**
  \param[in] mode if 1 the values higher than the threshold are given value 1.
                  If the mode is -1, the values lower the threshold are given 1.
**/
 IMPEM2DEXPORT void thresholding(
                  const algebra::Matrix2D_d &m,algebra::Matrix2D_d &result,
                   double threshold, int mode);


//! Applies a binary mask to an image.
/**
  \param[in] mask If the mask is 1, the result has the value of m. Otherwise
             the value is given by the parameter value.
  \param[in] value value to apply when the mask is 0
  \note  m and result can be the same matrix.

**/
IMPEM2DEXPORT void masking(
          const algebra::Matrix2D_d &m,algebra::Matrix2D_d &result,
          const algebra::Matrix2D<int> &mask,double value);



//! Smoothing filter by application of the reaction-diffusion
//! equation of Beltrami flow. Adiga, JSB, 2005
/**
  \param [in] beta contribution of diffusion versus edge enhancement.
              0 - pure reaction, 90 - pure diffusion
**/
 IMPEM2DEXPORT void diffusion_filtering(
             const algebra::Matrix2D_d &I,
             algebra::Matrix2D_d &result,
              double beta,
              double pixelsize,
              unsigned int t_steps);





template<typename T,typename U>
void transfer_by_casting(algebra::Matrix2D<T> &m,
                         algebra::Matrix2D<U> &result) {
  result.reshape(m);
  for (int i=m.get_start(0);i<=m.get_finish(0);++i) {
    for (int j=m.get_start(1);j<=m.get_finish(1);++j) {
      // cast
      result(i,j)=(U)m(i,j);
    }
  }
}


//! Removes small objects from a labeled image. The background value is assumed
//! to be 0, and the labels start at 1 up to the number of labels.
/**
  \param[in] m the matrix
  \param[in] percentage The percentage respect to the largest object that
             other objects have to be in order to survive the removal.
  \param[in] n_labels If higher than 0 (default) is the number of labels in the
             image. Otherwise, the function computes the number (slower)
**/
template<typename T>
void remove_small_objects(algebra::Matrix2D<T> &m,
            double percentage,int n_labels=0) {
  int background =0;
  int foreground =1;
  // If not given, compute the number of labels. Requires one scan
  if(n_labels== 0) {
    n_labels = algebra::get_rounded(m.compute_max());
  }
  // Vector to count pixels
  std::vector<unsigned int> pixel_count(n_labels);
  for (unsigned int i=0;i<pixel_count.size();++i) {
    pixel_count[i]=0;
  }
  // First scan, count pixels
  for (unsigned int i=0;i<m.num_elements();++i) {
    int val = algebra::get_rounded(m.data()[i]);
    if(val != background) {
      (pixel_count[val-1])++;
    }
  }
  // Get largest object
  unsigned int max_pixels = *max_element(pixel_count.begin(),pixel_count.end());
  std::vector<double> percentages(n_labels);
  for (unsigned int i=0;i<pixel_count.size();++i) {
    percentages[i]=((double)pixel_count[i])/((double)max_pixels);
  }
  // Second scan, remove objects of size lower than the percentage
  for (unsigned int i=0;i<m.num_elements();++i) {
    int val = algebra::get_rounded(m.data()[i]);
    if(percentages[val-1] > percentage) {
      m.data()[i] = (T)foreground;
    } else {
      m.data()[i] = (T)background;
    }
  }
}




//! (U. Adiga, 2005)
/**
  \param[in] m binary matrix to dilate and shrink
  \param[in] greyscale greyscale matrix that controls the shrinking
  \param[in] kernel dilation kernel
  \note Only tested with binary matrices m with background =0 and foreground = 1
**/
 IMPEM2DEXPORT void dilate_and_shrink_warp(algebra::Matrix2D_d &m,
                            const algebra::Matrix2D_d &greyscale,
                            algebra::Matrix2D_d &kernel);


 IMPEM2DEXPORT void histogram_stretching(algebra::Matrix2D_d &m,
                          int boxes,int offset);





//! Computes the histogram of a matrix.
/**
  \param[in] m Matrix with the data
  \param[in] bins Number of bins to use in the histogram
  \param[out] vector with the values for each bin
**/
IMPEM2DEXPORT Floats get_histogram(const cv::Mat &m, int bins);

IMPEM2DEXPORT Floats get_histogram(em2d::Image *img,int bins);


//! Variance filter for an image. Computes the variance for each pixel
/**
  \param[in] input image with the data
  \param[out] filtered result of the filtering
  \param[in] kernelsize The variance is computed using kernelsize x kernelsize
              pixels around each one. Kernelsize can only be odd.
**/
IMPEM2DEXPORT void apply_variance_filter(em2d::Image *input,
                                   em2d::Image *filtered,int kernelsize);

IMPEM2DEXPORT void apply_variance_filter(const cv::Mat &input,
                                   cv::Mat &filtered,int kernelsize);


IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_IMAGE_PROCESSING_H */
