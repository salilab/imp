/*!
 *  \file IMP/em2d/align2D.h
 *  \brief Alignment of images in 2D
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_ALIGN_2D_H
#define IMPEM2D_ALIGN_2D_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/em2d/PolarResamplingParameters.h"
#include "IMP/algebra/utility.h"
#include "IMP/algebra/Transformation2D.h"
#include "IMP/algebra/Vector2D.h"
#include <algorithm>
#include <vector>

IMPEM2D_BEGIN_NAMESPACE

//! Definition of the result of an alignment. Contains the transformation
//! to apply to the matrix to align and the cross correlation of the solution
typedef std::pair<algebra::Transformation2D, double> ResultAlign2D;

//! Aligns completely two matrices (rotationally and translationally).
//! Uses the autocorrelation function to speed up the rotational alignment
/*!
  \param[in] input Reference matrix
  \param[in] m_to_align Matrix to align to the reference
  \param[in] apply if true, apply the transformation
             to m_to_align after alignment
  \return The result. Check the definition of  ResultAlign2D
*/
IMPEM2DEXPORT ResultAlign2D get_complete_alignment(const cv::Mat &input,
                                                   cv::Mat &m_to_align,
                                                   bool apply = false);

//! Aligns two matrices rotationally. Based on the autocorrelation function
//! of the matrices.
/*!
  \param[in] input Reference matrix
  \param[in] m_to_align Matrix to align to the reference
  \param[in] apply if true, apply the transformation
             to m_to_align after alignment
  \return The result. Check the definition of  ResultAlign2D
*/
IMPEM2DEXPORT em2d::ResultAlign2D get_rotational_alignment(const cv::Mat &input,
                                                           cv::Mat &m_to_align,
                                                           bool apply = false);

//! Aligns two matrices translationally
/*!
  \param[in] input Reference matrix
  \param[in] m_to_align Matrix to align to the reference
  \param[in] apply if true, apply the transformation
             to m_to_align after alignment
  \return The result. Check the definition of  ResultAlign2D
*/
IMPEM2DEXPORT ResultAlign2D get_translational_alignment(const cv::Mat &input,
                                                        cv::Mat &m_to_align,
                                                        bool apply = false);

//! Aligns two matrices rotationally and translationally without
//! performing preprocessing. Preprocessed data must be provided.
/*!
  \param[in] input first matrix
  \param[in] INPUT fft of the first matrix
  \param[in] POLAR1 fft of the autocorrelation (in polars) of the input.
  \param[in] m_to_align the matrix to align with the input
  \param[in] POLAR2 fft of the autocorrelation (in polars) of m_to_align
  \param[in] apply true if m_to_align is transformed at the end
  \return The result. Check the definition of  ResultAlign2D
*/
IMPEM2DEXPORT em2d::ResultAlign2D get_complete_alignment_no_preprocessing(
    const cv::Mat &input, const cv::Mat &INPUT, const cv::Mat &POLAR1,
    cv::Mat &m_to_align, const cv::Mat &POLAR2, bool apply = false);

//! Computes the rotational alignment for two autocorrelation matrices
//! It is assumed that the arguments are FFTs
/*!
  \param[in]  POLAR1 fft of the first autocorrelation (in polars)
  \param[in] POLAR2 fft of the second autocorrelation (in polars)
  \return The result. Check the definition of  ResultAlign2D
*/
IMPEM2DEXPORT ResultAlign2D
    get_rotational_alignment_no_preprocessing(const cv::Mat &POLAR1,
                                              const cv::Mat &POLAR2);

//! Aligns two matrices translationally without preprocessing. The preprocessed
//! data must be provided.
/*!
  \param[in]  M1 the dft the first matrix (input)
  \param[in]  M2 the dft of the matrix to align with the input
  \return The result. Check the definition of  ResultAlign2D
*/
IMPEM2DEXPORT ResultAlign2D
    get_translational_alignment_no_preprocessing(const cv::Mat &M1,
                                                 const cv::Mat &M2);

//! Aligns two matrices (rotation and translation) using centers and
//! no preprocessing. Preprocessed data must be provided.
/*!
  \param[in] center1  weighted centroid of the reference matrix
  \param[in] center2  weighted centroid of the matrix to align
  \param[in] AUTOC_POLAR1 fft of the autocorrelation (in polars) of the input.
  \param[in] AUTOC_POLAR2 fft of the autocorrelation (in polars) of m_to_align
  \return The result. Check the definition of  ResultAlign2D
*/
IMPEM2DEXPORT ResultAlign2D
    get_complete_alignment_with_centers_no_preprocessing(
        const algebra::Vector2D &center1, const algebra::Vector2D &center2,
        const cv::Mat &AUTOC_POLAR1, const cv::Mat &AUTOC_POLAR2);

IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_ALIGN_2D_H */
