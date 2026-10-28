/**
 *  \file align2D.cpp
 *  \brief Align operations for EM images
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
*/

#include "IMP/em2d/align2D.h"
#include "IMP/em2d/FFToperations.h"
#include "IMP/em2d/image_processing.h"
#include "IMP/em2d/Matrix.h"
#include "IMP/em2d/scores2D.h"
#include "IMP/em2d/internal/image_processing_helper.h"
#include "IMP/em/SpiderReaderWriter.h"
#include "IMP/em/exp.h"
#include "IMP/exception.h"
#include "IMP/constants.h"
#include <complex>
#include <cmath>

IMPEM2D_BEGIN_NAMESPACE

ResultAlign2D get_complete_alignment(const DoubleMatrix &input,
                                     DoubleMatrix &m_to_align,
                                     bool apply) {
  IMP_LOG_TERSE("starting complete 2D alignment " << std::endl);
  DoubleMatrix autoc1, autoc2, aux1, aux2, aux3;
  algebra::Transformation2D transformation1, transformation2;
  ResultAlign2D RA;
  get_autocorrelation2d(input, autoc1);
  get_autocorrelation2d(m_to_align, autoc2);
  RA = get_rotational_alignment(autoc1, autoc2, false);
  double angle1 = RA.first.get_rotation().get_angle();
  get_transformed(m_to_align, aux1, RA.first);  // rotate
  RA = get_translational_alignment(input, aux1);
  algebra::Vector2D shift1 = RA.first.get_translation();
  transformation1.set_rotation(angle1);
  transformation1.set_translation(shift1);
  get_transformed(m_to_align, aux2, transformation1);
  double ccc1 = get_cross_correlation_coefficient(input, aux2);
  // Check for both angles that can be the solution
  double angle2;
  if (angle1 < PI) {
    angle2 = angle1 + PI;
  } else {
    angle2 = angle1 - PI;
  }
  // rotate
  algebra::Rotation2D R2(angle2);
  algebra::Transformation2D tr(R2);
  get_transformed(m_to_align, aux3, tr);

  RA = get_translational_alignment(input, aux3);
  algebra::Vector2D shift2 = RA.first.get_translation();
  transformation2.set_rotation(angle2);
  transformation2.set_translation(shift2);
  get_transformed(m_to_align, aux3, transformation2);
  double ccc2 = get_cross_correlation_coefficient(input, aux3);
  if (ccc2 > ccc1) {
    if (apply) {
      m_to_align = aux3;
    }
    IMP_LOG_VERBOSE(" Transformation= " << transformation2
                                        << " cross_correlation = " << ccc2
                                        << std::endl);
    return em2d::ResultAlign2D(transformation2, ccc2);
  } else {
    if (apply) {
      m_to_align = aux2;
    }
    IMP_LOG_VERBOSE(" Transformation= " << transformation1
                                        << " cross_correlation = " << ccc1
                                        << std::endl);
    return em2d::ResultAlign2D(transformation1, ccc1);
  }
}

ResultAlign2D get_rotational_alignment(const DoubleMatrix &input,
                                       DoubleMatrix &m_to_align, bool apply) {
  IMP_LOG_TERSE("starting 2D rotational alignment" << std::endl);
  IMP_USAGE_CHECK(
      (input.rows() == m_to_align.rows()) &&
          (input.cols() == m_to_align.cols()),
      "em2d::align_rotational: Matrices have different size.");

  DoubleMatrix polar1, polar2, corr;
  // Build maps for resampling
  PolarResamplingParameters polar_params(input.rows(), input.cols());
  polar_params.set_estimated_number_of_angles(std::min(input.rows(),
                                                       input.cols()));

  polar_params.create_maps_for_resampling();
  do_resample_polar(input, polar1, polar_params);       // subject image
  do_resample_polar(m_to_align, polar2, polar_params);  // projection image

  ResultAlign2D RA = get_translational_alignment(polar1, polar2);
  algebra::Vector2D shift = RA.first.get_translation();
  // column shift[0] is the optimal angle to bring m_to_align to input
  double angle = shift[0] * polar_params.get_angle_step();
  RA.first.set_rotation(angle);
  RA.first.set_translation(algebra::Vector2D(0.0, 0.0));
  // Apply the rotation if requested
  if (apply) {
    DoubleMatrix result;
    get_transformed(m_to_align, result, RA.first);
    m_to_align = result;
  }
  IMP_LOG_VERBOSE("Rotational alignment: Transformation= "
                  << RA.first << " cross_correlation = " << RA.second
                  << std::endl);
  return RA;
}

ResultAlign2D get_translational_alignment(const DoubleMatrix &input,
                                          DoubleMatrix &m_to_align,
                                          bool apply) {
  IMP_LOG_TERSE("starting 2D translational alignment" << std::endl);
  IMP_USAGE_CHECK(
      (input.rows() == m_to_align.rows()) &&
          (input.cols() == m_to_align.cols()),
      "em2d::align_translational: Matrices have different size.");
  DoubleMatrix corr;
  get_correlation2d(input, m_to_align, corr);
  double max_cc;
  algebra::Vector2D peak = internal::get_peak(corr, &max_cc);
  // Convert the pixel with the maximum to a shift respect to the center
  algebra::Vector2D shift(peak[0] - static_cast<double>(corr.cols()) / 2.,
                          peak[1] - static_cast<double>(corr.rows()) / 2.);
  algebra::Transformation2D t(shift);
  // Apply the shift if requested
  if (apply) {
    DoubleMatrix result;
    get_transformed(m_to_align, result, t);
    m_to_align = result;
  }
  IMP_LOG_VERBOSE(" Transformation= " << t << " cross_correlation = " << max_cc
                                      << std::endl);
  return ResultAlign2D(t, max_cc);
}

ResultAlign2D get_complete_alignment_no_preprocessing(
    const DoubleMatrix &input, const DoubleMatrix &INPUT,
    const DoubleMatrix &POLAR1, DoubleMatrix &m_to_align,
    const DoubleMatrix &POLAR2, bool apply) {

  IMP_LOG_TERSE("starting complete 2D alignment with no preprocessing"
                << std::endl);

  DoubleMatrix aux1, aux2, aux3, aux4;  // auxiliary matrices
  DoubleMatrix AUX1, AUX2, AUX3;        // ffts
  algebra::Transformation2D transformation1, transformation2;
  double angle1 = 0, angle2 = 0;
  ResultAlign2D RA = get_rotational_alignment_no_preprocessing(POLAR1, POLAR2);
  angle1 = RA.first.get_rotation().get_angle();
  get_transformed(m_to_align, aux1, RA.first);  // rotate
  get_fft_using_optimal_size(aux1, AUX1);
  RA = get_translational_alignment_no_preprocessing(INPUT, AUX1);
  algebra::Vector2D shift1 = RA.first.get_translation();
  transformation1.set_rotation(angle1);
  transformation1.set_translation(shift1);
  get_transformed(m_to_align, aux2, transformation1);  // rotate
  double ccc1 = get_cross_correlation_coefficient(input, aux2);
  // Check the opposed angle
  if (angle1 < PI) {
    angle2 = angle1 + PI;
  } else {
    angle2 = angle1 - PI;
  }
  algebra::Rotation2D R2(angle2);
  algebra::Transformation2D tr(R2);
  get_transformed(m_to_align, aux3, tr);  // rotate
  get_fft_using_optimal_size(aux3, AUX3);

  RA = get_translational_alignment_no_preprocessing(INPUT, AUX3);
  algebra::Vector2D shift2 = RA.first.get_translation();
  transformation2.set_rotation(angle2);
  transformation2.set_translation(shift2);
  get_transformed(m_to_align, aux3, transformation2);
  double ccc2 = get_cross_correlation_coefficient(input, aux3);

  if (ccc2 > ccc1) {
    if (apply) {
      m_to_align = aux3;
    }
    IMP_LOG_VERBOSE(" Align2D complete Transformation= "
                    << transformation2 << " cross_correlation = " << ccc2
                    << std::endl);
    return ResultAlign2D(transformation2, ccc2);
  } else {
    if (apply) {
      m_to_align = aux3;
    }
    IMP_LOG_VERBOSE(" Align2D complete Transformation= "
                    << transformation1 << " cross_correlation = " << ccc1
                    << std::endl);
    return ResultAlign2D(transformation1, ccc1);
  }
}

ResultAlign2D get_rotational_alignment_no_preprocessing(
    const DoubleMatrix &POLAR1, const DoubleMatrix &POLAR2) {
  IMP_LOG_TERSE("starting 2D rotational alignment with no preprocessing"
                << std::endl);

  IMP_USAGE_CHECK(
      ((POLAR1.rows() == POLAR2.rows()) && (POLAR1.cols() == POLAR2.cols())),
      "get_rotational_alignment_no_preprocessing: Matrices have different "
      "size.");

  ResultAlign2D RA =
      get_translational_alignment_no_preprocessing(POLAR1, POLAR2);
  algebra::Vector2D shift = RA.first.get_translation();
  // The number of columns of the polar matrices
  // are the number of angles considered. Init a PolarResamplingParameters
  // to get the angle_step
  PolarResamplingParameters polar_params;
  polar_params.set_estimated_number_of_angles(POLAR1.cols());
  double angle = shift[0] * polar_params.get_angle_step();
  RA.first.set_rotation(angle);
  RA.first.set_translation(algebra::Vector2D(0.0, 0.0));
  IMP_LOG_VERBOSE("Rotational Transformation= "
                  << RA.first << " cross_correlation = " << RA.second
                  << std::endl);
  return RA;
}

ResultAlign2D get_translational_alignment_no_preprocessing(
    const DoubleMatrix &M1, const DoubleMatrix &M2) {
  IMP_LOG_TERSE("starting 2D translational alignment with no preprocessing"
                << std::endl);
  IMP_USAGE_CHECK(((M1.rows() == M2.rows()) && (M1.cols() == M2.cols())),
                  "get_translational_alignment_no_preprocessing: "
                  "Matrices have different size.");

  DoubleMatrix corr;
  corr.resize(M1.rows(), M1.cols());
  get_correlation2d_no_preprocessing(M1, M2, corr);  // corr must be allocated!
  // Find the peak of the cross_correlation
  double max_cc;
  algebra::Vector2D peak = internal::get_peak(corr, &max_cc);

  // Convert the pixel with the maximum to a shift respect to the center
  algebra::Vector2D shift(peak[0] - static_cast<double>(corr.cols()) / 2.,
                          peak[1] - static_cast<double>(corr.rows()) / 2.);
  algebra::Transformation2D t(shift);
  IMP_LOG_VERBOSE(" Translational Transformation = "
                  << t << " cross_correlation = " << max_cc << std::endl);
  return ResultAlign2D(t, max_cc);
}

ResultAlign2D get_complete_alignment_with_centers_no_preprocessing(
    const algebra::Vector2D &center1, const algebra::Vector2D &center2,
    const DoubleMatrix &AUTOC_POLAR1, const DoubleMatrix &AUTOC_POLAR2) {
  // Align rotationally with FFT
  ResultAlign2D RA =
      get_rotational_alignment_no_preprocessing(AUTOC_POLAR1, AUTOC_POLAR2);
  double angle = RA.first.get_rotation().get_angle();
  if (angle < 0) {
    angle += 2 * PI;
  }
  // Compute translation using the centers
  algebra::Rotation2D R(angle);
  algebra::Vector2D displacement = center1 - R.get_rotated(center2);
  algebra::Transformation2D t(R, displacement);
  return ResultAlign2D(t, RA.second);
}

IMPEM2D_END_NAMESPACE
