/*!
 *  \file scores2D.h
 *  \brief Scoring functions for 2D
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_SCORES_2D_H
#define IMPEM2D_SCORES_2D_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/em2d/Image.h"
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




/*

//! Base class for all scoring functions related to em2d
class IMPEM2DEXPORT Score {
public:


  //! Given an image and a projection, returns the appropiate score
  double operator()(Image *image,Image *projection) const {
    // trying to use the non-virtual interface (Alexandrescu, 39)
    return get_score(image,projection);
  }

  virtual ~Score();
private:
  double get_score(Image *image,Image *projection) const = 0;

};


//! Score based on Chi^2 = ((pixels_iamge - pixels_projection)/stddev_image)^2
class IMPEM2DEXPORT ChiSquaredScore: public Score {
public:
  ChiSquaredScore() {};

private:
  double get_score(Image *image,Image *projection) const {
    return 0.0;
  }

};





//! Score based on squared differences (pixels_iamge - pixels_projection)**2
class IMPEM2DEXPORT SquaredDifferenceScore: public Score {
public:
  SquaredDifferenceScore() {};

private:
  double get_score(Image *image,Image *projection) const {
    return 0.0;
  }
};


//! Score based on cross correlation
class IMPEM2DEXPORT CCCScore: public Score {
public:
  CCCScore() {};

private:
  double get_score(Image *image,Image *projection) const {
    return 0.0;
  }
};


*/

IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_SCORES_2D_H */
