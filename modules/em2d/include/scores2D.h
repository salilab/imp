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
#include "IMP/Object.h"
#include "IMP/VersionInfo.h"
// #include <boost/shared_ptr.hpp>

IMPEM2D_BEGIN_NAMESPACE

class ScoreFunction;
// typedef boost::shared_ptr<ScoreFunction> ScoreFunctionPtr;

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






//! Base class for all scoring functions related to em2d
class IMPEM2DEXPORT ScoreFunction: public Object {
public:
  ScoreFunction() {};
  //! Given an image and a projection, returns the appropiate score
  double get_score(Image *image,Image *projection) const {
    // trying to use the non-virtual interface (Alexandrescu, 39)
    return get_private_score(image,projection);
  }
  void show(std::ostream &out) const {};
  IMP_OBJECT_INLINE(ScoreFunction, show(out), {});
private:
  virtual double get_private_score(Image *image,Image *projection) const = 0;
};
IMP_OBJECTS(ScoreFunction,ScoreFunctions);
IMP_OUTPUT_OPERATOR(ScoreFunction);



//! Score based on Chi^2 = ((pixels_iamge - pixels_projection)/stddev_image)^2
class IMPEM2DEXPORT ChiSquaredScore: public ScoreFunction {
public:
  ChiSquaredScore() {};
  void show(std::ostream &out) const {};

private:
  double get_private_score(Image *image,Image *projection) const {
    return 0.0;
  }
};
IMP_OBJECTS(ChiSquaredScore,ChiSquaredScores);
IMP_OUTPUT_OPERATOR(ChiSquaredScore);


//! Score based on cross correlation
class IMPEM2DEXPORT CCCScore: public ScoreFunction {
public:
  CCCScore() {};
  void show(std::ostream &out) const {};
private:
  double get_private_score(Image *image,Image *projection) const {
    return get_cross_correlation_coefficient(image->get_data(),
                                             projection->get_data());
  }
};
IMP_OBJECTS(CCCScore,CCCScores);
IMP_OUTPUT_OPERATOR(CCCScore);



//! EM2DScore, based on squared differences
//!  (pixels_iamge - pixels_projection)**2
class IMPEM2DEXPORT EM2DScore: public ScoreFunction {
public:
  EM2DScore() {
    IMP_LOG(IMP::VERBOSE,"EM2DScore function created" << std::endl);
};
  void show(std::ostream &out) const {
    cout << "---> EM2DScore score_function." << std::endl;};

private:
  double get_private_score(Image *image,Image *projection) const {
    IMP_LOG(IMP::TERSE,"I am scoring from EM2DScore" << std::endl);
    return 1-get_cross_correlation_coefficient(image->get_data(),
                                             projection->get_data());
  }
};
IMP_OBJECTS(EM2DScore,EM2DScores);
IMP_OUTPUT_OPERATOR(EM2DScore);




// Comparison by value of the ccc
template<class T>
class HasHigherCCC
#ifndef SWIG
:
    public std::binary_function< T , T ,bool>
#endif
 {
public:
  bool operator()(const T &a,const T &b) { return a.get_ccc() >= b.get_ccc();}
  void show(std::ostream &out) const {};
};


// Comparison of pairs by checking the second element
template<class T>
class LessPairBySecond
#ifndef SWIG
:
   public std::binary_function< T ,T , bool>
#endif
 {
public:
  bool operator()(const T &a,const T &b) { return a.second < b.second;}
  void show(std::ostream &out) const {};
};



IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_SCORES_2D_H */
