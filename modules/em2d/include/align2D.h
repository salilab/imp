/*!
 *  \file align2D.h
 *  \brief Alignment of images in 2D
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_ALIGN_2D_H
#define IMPEM2D_ALIGN_2D_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/em2d/PolarResamplingParameters.h"
#include "IMP/em/image_transformations.h"
#include "IMP/algebra/utility.h"
#include "IMP/algebra/Matrix2D.h"
#include "IMP/algebra/Transformation2D.h"
#include "IMP/algebra/Vector2D.h"
#include <algorithm>
#include <vector>

IMPEM2D_BEGIN_NAMESPACE

//! Definition of the result of an alignment. Contains the transformation
//! to apply to the matrix to align and the cross correlation of the solution
typedef std::pair<algebra::Transformation2D,double> ResultAlign2D;


//! Aligns completely two matrices (rotationally and translationally).
//! Uses the autocorrelation function to speed up the rotational alignment
/*!
  \param[in] input Reference matrix
  \param[in] m_to_align Matrix to align to the reference
  \param[in] apply if true, apply the transformation
             to m_to_align after alignment
  \param[out] The result. Check the definition of  ResultAlign2D
*/
IMPEM2DEXPORT ResultAlign2D align2D_complete(const cv::Mat &input,
          cv::Mat &m_to_align,bool apply=false);

//! Aligns two matrices rotationally. Based on the autocorrelation function
//! of the matrices.
/*!
  \param[in] input Reference matrix
  \param[in] m_to_align Matrix to align to the reference
  \param[in] apply if true, apply the transformation
             to m_to_align after alignment
  \param[out] The result. Check the definition of  ResultAlign2D
*/
IMPEM2DEXPORT em2d::ResultAlign2D align2D_rotational(const cv::Mat &input,
                          cv::Mat &m_to_align,bool apply=false);


//! Aligns two matrices translationally
/*!
  \param[in] input Reference matrix
  \param[in] m_to_align Matrix to align to the reference
  \param[in] apply if true, apply the transformation
             to m_to_align after alignment
  \param[out] The result. Check the definition of  ResultAlign2D
*/
IMPEM2DEXPORT ResultAlign2D align2D_translational(const cv::Mat &input,
                           cv::Mat &m_to_align,bool apply=false);


//! Aligns two matrices rotationally and translationally without
//! performing preprocessing. Preprocessed data must be provided.
/*!
  \param[in] input first matrix
  \param[in] INPUT fft of the first matrix
  \param[in] POLAR1 fft of the autocorrelation (in polars) of the input.
  \param[in] m_to_align the matrix to align with the input
  \param[in] POLAR2 fft of the autocorrelation (in polars) of m_to_align
  \param[in] apply true if m_to_align is transformed at the end
  \param[out] The result. Check the definition of  ResultAlign2D
*/
IMPEM2DEXPORT em2d::ResultAlign2D align2D_complete_no_preprocessing(
                const cv::Mat &input,
                const cv::Mat &INPUT,const cv::Mat &POLAR1,
                cv::Mat &m_to_align,const cv::Mat &POLAR2,bool apply=false);

//! Computes the rotational alignment for two autocorrelation matrices
//! It is assumed that the arguments are FFTs
/*!
  \param[in]  M1 fft of the first autocorrelation (in polars)
  \param[in] M2 fft of the second autocorrelation (in polars)
  \param[out] The result. Check the definition of  ResultAlign2D
*/
IMPEM2DEXPORT ResultAlign2D align2D_rotational_no_preprocessing(
                                                  const cv::Mat &POLAR1,
                                                  const cv::Mat &POLAR2);

//! Aligns two matrices translationally without preprocessing. The preprocessed
//! data must be provided.
/*!
  \param[in]  M1 the dft the first matrix (input)
  \param[in]  M2 the dft of the matrix to align with the input
  \param[out] The result. Check the definition of  ResultAlign2D
*/
IMPEM2DEXPORT ResultAlign2D align2D_translational_no_preprocessing(
                                                     const cv::Mat &M1,
                                                     const cv::Mat &M2);


//! Performs a peak search in a matrix
/*!
  \param[in] m matrix
  \param[out] value the value at the peak
  \param[out] the position of the peak. Subpixel precision, using interpolation
*/
IMPEM2DEXPORT algebra::Vector2D peak_search(cv::Mat &m,double *value);


//! computes the weighted centroid of a matrix, with the values of the matrix
IMPEM2DEXPORT algebra::Vector2D get_weighted_centroid(const cv::Mat &m);


IMPEM2DEXPORT void print_vector(std::vector< std::complex<double> > &v);


//! Aligns two matrices (rotation and translation) using centers and
//! no preprocessing. Preprocessed data must be provided.
/*!
  \param[in] center1  weighted centroid of the reference matrix
  \param[in] center2  weighted centroid of the matrix to align
  \param[in] AUTOC_POLAR1 fft of the autocorrelation (in polars) of the input.
  \param[in] AUTOC_POLAR2 fft of the autocorrelation (in polars) of m_to_align
  \param[out] The result. Check the definition of  ResultAlign2D
*/
IMPEM2DEXPORT ResultAlign2D align2D_complete_with_centers_no_preprocessing(
                      const algebra::Vector2D &center1,
                      const algebra::Vector2D &center2,
                      const cv::Mat &AUTOC_POLAR1,
                      const cv::Mat &AUTOC_POLAR2);


IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_ALIGN_2D_H */
