/**
 *  \file scores2D.h
 *  \brief Scoring functions for 2D
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_SCORES_2D_H
#define IMPEM2D_SCORES_2D_H

#include "IMP/em/Image.h"
#include "IMP/em2d/RegistrationResult.h"


IMPEM2D_BEGIN_NAMESPACE


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

IMPEM2DEXPORT double ccc_to_em2d(double ccc);
IMPEM2DEXPORT double em2d_to_ccc(double em2d);

IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_SCORES_2D_H */
