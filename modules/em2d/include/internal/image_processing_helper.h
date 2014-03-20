/**
 *  \file image_processing_helper.h
 *  \brief
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM2D_INTERNAL_IMAGE_PROCESSING_HELPER_H
#define IMPEM2D_INTERNAL_IMAGE_PROCESSING_HELPER_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/em2d/opencv_interface.h"
#include "IMP/algebra/Vector2D.h"

IMPEM2D_BEGIN_INTERNAL_NAMESPACE

const int NO_VALUE = std::numeric_limits<int>::max();

//! Returns the neighbors for a givel pixel in a given mode
/*!
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
*/
cvPixels get_neighbors2d(const cvPixel &p, const cv::Mat &m, int mode = 4,
                         int sign = 0, bool cycle = false);

/***************************

FUNCTIONS FOR THE LABELING ALGORITHM

***************************/

void set_final_labels(cvIntMat &mat_to_label, Ints &Labels);

//! union operation. Joins trees containing nodes i and j and returns the
//! the new root
/*!
  \param[in] P vector of labels
  \param[in] i a node
  \param[in] j another node
  \param[out] root the new root
*/
int do_union(Ints &Labels, int i, int j);

//! Sets a new label for a pixel
int get_new_label(const cvPixel &p, cvIntMat &mat_to_label, Ints &Labels,
                  int label);

//! Decission tree copy function
void do_copy_tree(const cvPixel &p, const cvPixel &a, cvIntMat &mat_to_label);

//! Decission tree copy function
void do_copy_tree(const cvPixel &p, const cvPixel &a, const cvPixel &c,
                  cvIntMat &mat_to_label, Ints &Labels);

//! find root operation
/*!
  \param[in] P vector of labels
  \param[in] i a node
  \param[out] root root of the tree for i
*/
int find_root(const Ints &Labels, int i);

//! Set root operation
/*!
  \param[in] P vector of labels
  \param[in] i node of a tree
  \param[out] root root node of the tree of i
*/
void set_root(Ints &Labels, int i, int root);

//! flattens the vector with the union-find tree of labels and
//! relabels the components
void do_flatten_labels(Ints &Labels);

//! Decission tree for neighbor exploration
int get_label_using_decission_tree(const cvPixel &p, cvPixels &neighbors,
                                   int background, const cvIntMat &m,
                                   cvIntMat &mat_to_label, Ints &Labels,
                                   int label);

//! SCT1 Scanning function
/*!
  \param[in] m binary matrix to scan
  \param[out] mat_to_label matrix with the labels
*/
void do_binary_matrix_scan(const cvIntMat &m, cvIntMat &mat_to_label,
                           Ints &Labels);

//! Performs a peak search in a matrix
/*!
  \param[in] m matrix
  \param[out] value the value at the peak
  \return the position of the peak. Subpixel precision, using interpolation
*/
algebra::Vector2D get_peak(cv::Mat &m, double *value);

//! computes the weighted centroid of a matrix, with the values of the matrix
algebra::Vector2D get_weighted_centroid(const cv::Mat &m);

IMPEM2D_END_INTERNAL_NAMESPACE

#endif /* IMPEM2D_INTERNAL_IMAGE_PROCESSING_HELPER_H */
