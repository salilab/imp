/**
 *  \file scores2D.h
 *  \brief Scoring functions for 2D
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_SCORES_2D_H
#define IMPEM2D_SCORES_2D_H

#include "IMP/em/Image.h"
#include "IMP/em2D/RegistrationResult.h"


IMPEM2D_BEGIN_NAMESPACE

//! Computes the discrepancy score for two sets of Images. The order is
//! important. The second set containts the equivalent images for the first.
/**
  \param[in] subjects - Set of images (typically subject images).
  \param[in] projection - Set of images (typically projections) to compare with
    the first set, with equivalent ones given in the same order.
  \param[in] apply_transformations If true, the 2D transformations required
             to 2D align each projection to each subject (subject images are
             never modified) are applied.
  \note the score computed is the sum of the squared norms of the difference
  images, divided by the number of subjects.
**/
IMPEM2DEXPORT double discrepancy_scores(
                           const em::Images &subjects,
                           const em::Images &projections,
                           bool apply_transformations=true);


//! Computes the discrepancy score for two images.
/**
  \param[in] subject - subject image
  \param[in] projection - image to compare with the first
  \param[in] apply_transformations If true, the 2D transformation required
             to align the projection to the subject (subject image is
             never modified) is applied.
  \note the score computed is the sum of the squared norms of the difference
  image, divided by the size.
**/
IMPEM2DEXPORT double discrepancy_score(em::Image &subject,
                         em::Image &projection,
                         bool apply_transformations=true);


//! angle in the axis-angle representation of the rotation
//! that takes first to second
IMPEM2DEXPORT double rotation_error(RegistrationResult &rr1,
                                    RegistrationResult &rr2);


//! Distance between the two in-plane translations
IMPEM2DEXPORT double shift_error(RegistrationResult &rr1,
                                          RegistrationResult &rr2);

IMPEM2DEXPORT double average_rotation_error(RegistrationResults correct_RRs,
                                            RegistrationResults computed_RRs);

IMPEM2DEXPORT double average_shift_error(RegistrationResults correct_RRs,
                                         RegistrationResults computed_RRs);

//! Converts the cross-correlation coefficient in the em2D score that is
//! used for the discrepancy and vice-versa
inline double ccc_to_em2D_score(double ccc) {return 1-ccc;}
inline double em2D_score_to_ccc(double em2D) {return 1-em2D;}


IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_SCORES_2D_H */
