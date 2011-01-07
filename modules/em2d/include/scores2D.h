/*!
 *  \file scores2D.h
 *  \brief Scoring functions for 2D
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_SCORES_2D_H
#define IMPEM2D_SCORES_2D_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/em2d/opencv_interface.h"
#include "IMP/em2d/RegistrationResult.h"

IMPEM2D_BEGIN_NAMESPACE

//! angle in the axis-angle representation of the rotation
//! that takes first to second
IMPEM2DEXPORT double get_rotation_error(const RegistrationResult &rr1,
                                    const RegistrationResult &rr2);


//! Distance between the two in-plane translations
IMPEM2DEXPORT double get_shift_error(const RegistrationResult &rr1,
                                 const RegistrationResult &rr2);

IMPEM2DEXPORT double get_average_rotation_error(
                                      const RegistrationResults &correct_RRs,
                                      const RegistrationResults &computed_RRs);

IMPEM2DEXPORT double get_average_shift_error(
                                   const RegistrationResults &correct_RRs,
                                   const RegistrationResults &computed_RRs);

//! Converts the cross-correlation coefficient into the em2d score
IMPEM2DEXPORT double get_ccc_to_em2d(double ccc);

//! Converts the em2d score into the cross-correlation coefficient
IMPEM2DEXPORT double get_em2d_to_ccc(double em2d);

//! Computes the cross-correlation coefficient between to matrices
IMPEM2DEXPORT double get_cross_correlation_coefficient(const cv::Mat &m1,
                                                   const cv::Mat &m2);

//! Get the em2d score given a set of registration results from images
IMPEM2DEXPORT double get_em2d_score(const RegistrationResults &RRs);


IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_SCORES_2D_H */
