/*!
 *  \file scores2D.h
 *  \brief Scoring functions for 2D
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_SCORES_2D_H
#define IMPEM2D_SCORES_2D_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/em2d/Image.h"
#include "IMP/em2d/opencv_interface.h"
#include "IMP/em2d/RegistrationResult.h"
#include "IMP/Object.h"
#include "IMP/VersionInfo.h"
#include <IMP/base/warning_macros.h>
#include <functional>

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

//! Computes the cross-correlation coefficient between to matrices
IMPEM2DEXPORT double get_cross_correlation_coefficient(const cv::Mat &m1,
                                                   const cv::Mat &m2);

//! Get the gloal score given a set of individual registration results
//! from images
/*!
  \note The function checks what time of registration results are given.
        - If the registration results correspond to a coarse registration,
        the score is based on the cross-correlation (currently, the score
        is a mean of the cross-correlation coefficients.
        - If the registration results are obtained after a fine registration,
        the score is the average of all the registration scores. This score
g        depends on the function selected. Eg. EM2DScore.
*/
IMPEM2DEXPORT double get_global_score(const RegistrationResults &RRs);


//! Base class for all scoring functions related to em2d
class IMPEM2DEXPORT ScoreFunction: public IMP::base::Object {
public:
  ScoreFunction(): Object("ScoreFunction%1%") {}

  //! Given an image and a projection, returns the appropiate score
  double get_score(Image *image, Image *projection) const {
    // trying to use the non-virtual interface (Alexandrescu, 39)
    return get_private_score(image, projection);
  }

  void set_variance_image(Image *var) {
    set_variance_image_private(var);
  }

  IMP_OBJECT_METHODS(ScoreFunction);

protected:
  // Number of particle images used to get the class average
  unsigned int n_members_;

private:
  virtual double get_private_score(Image *image, Image *projection) const = 0;
  virtual void set_variance_image_private(Image *image) {IMP_UNUSED(image);};


};
IMP_OBJECTS(ScoreFunction,ScoreFunctions);

//! Score based on Chi^2 = ((pixels_iamge - pixels_projection)/stddev_image)^2
class IMPEM2DEXPORT ChiSquaredScore: public ScoreFunction {
public:
  ChiSquaredScore(): ScoreFunction() {}

private:
  mutable Pointer<Image> variance_;
  double get_private_score(Image *, Image *) const;
  void set_variance_imag_private(Image *var) {variance_ = var;}
};
IMP_OBJECTS(ChiSquaredScore,ChiSquaredScores);


//! EM2DScore, based on squared differences
//!  (pixels_iamge - pixels_projection)**2
class IMPEM2DEXPORT EM2DScore: public ScoreFunction {
public:
  EM2DScore(): ScoreFunction() {}
private:
  double get_private_score(Image *image, Image *projection) const {
    return 1 - get_cross_correlation_coefficient(image->get_data(),
                                             projection->get_data());
  }
};
IMP_OBJECTS(EM2DScore,EM2DScores);

class IMPEM2DEXPORT MeanAbsoluteDifference: public ScoreFunction {
public:
  MeanAbsoluteDifference(): ScoreFunction() {}
private:
  double get_private_score(Image *image, Image *projection) const;
};
IMP_OBJECTS(MeanAbsoluteDifference, MeanAbsoluteDifferences);


//! Comparison by value of the ccc
template<class T>
class HasHigherCCC
#ifndef SWIG
:
    public std::binary_function< T , T ,bool>
#endif
 {
public:
  bool operator()(const T &a, const T &b) const {
    return a.get_ccc() >= b.get_ccc();
  }
  void show(std::ostream &) const {}
};


//! Comparison of pairs by checking the second element
template<class T>
class LessPairBySecond
#ifndef SWIG
:
   public std::binary_function< T, T, bool>
#endif
 {
public:
  bool operator()(const T &a, const T &b) const {
    return a.second < b.second;
  }
  void show(std::ostream &) const {}
};



//! Compare two classes that return a score
template<class T>
class HasLowerScore
#ifndef SWIG
:
    public std::binary_function< T , T ,bool>
#endif
 {
public:
  bool operator()(const T &a,const T &b) const {
    return a.get_score() < b.get_score();
  }
  void show(std::ostream &) const {}
};

IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_SCORES_2D_H */
