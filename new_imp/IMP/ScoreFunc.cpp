/*
 *  ScoreFunc.cpp
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include "ScoreFunc.h"
#include "log.h"
#include "mystdexcept.h"

namespace imp
{

/** Destructor */
Harmonic::~Harmonic()
{
  IMP_LOG(VERBOSE,
	  "Delete Harmonic: beware of early Python calls to destructor.");
}

/**
  Calculate harmonic score with respect to the given feature.

  \param[in] feature Value of feature being tested.

  \param[in] mean Expected mean value of the feature.

  \param[in] sd Allowable standard deviation value of the feature.
 */

Float Harmonic::operator()(Float feature)
{
  return harmonic(feature);
}

/**
  Calculate harmonic score and partial derivative of the score
  with respect to the given feature.

  \param[in] feature Value of feature being tested.

  \param[in] mean Expected mean value of the feature.

  \param[in] sd Allowable standard deviation value of the feature.

  \param[out] deriv Partial derivative of the score with respect to
                      the feature value.
 */

Float Harmonic::operator()(Float feature, Float& deriv)
{
  return harmonic(feature, deriv);
}

/**
  Calculate harmonic score with respect to the given feature.
  Implemented to support lower and upper bound harmonics as well.

  \param[in] feature Value of feature being tested.

  \param[in] mean Expected mean value of the feature.

  \param[in] sd Allowable standard deviation value of the feature.
 */

Float Harmonic::harmonic(Float feature)
{
  Float dummy;

  return harmonic(feature, dummy);
}

/**
  Calculate harmonic score and partial derivative of the score
  with respect to the given feature. Implemented to support lower
  and upper bound harmonics as well.

  \param[in] feature Value of feature being tested.

  \param[in] mean Expected mean value of the feature.

  \param[in] sd Allowable standard deviation value of the feature.

  \param[out] deriv Partial derivative of the score with respect to
                      the feature value.
 */

Float Harmonic::harmonic(Float feature, Float& deriv)
{
  Float e;
  Float sd;
  
  sd = sd_ / 0.54318463; // correction factor to match Modeller
  e = (feature - mean_) / sd;
  deriv = e / sd * 2.0; // * 2.0 is correction factor to match Modeller
  return e * e;
}

/** Destructor */
HarmonicLowerBound::~HarmonicLowerBound()
{
  IMP_LOG(VERBOSE, "Delete HarmonicLowerBound: beware of early Python "
	  << "calls to destructor.");
}

/**
  Calculate harmonic score with respect to the given feature if
  the feature is less than the mean. Otherwise, the score is zero.

  \param[in] feature Value of feature being tested.

  \param[in] mean Expected mean value of the feature.

  \param[in] sd Allowable standard deviation value of the feature.
 */

Float HarmonicLowerBound::operator()(Float feature)
{
  if (feature >= mean_)
    return 0.0;
  else
    return harmonic(feature);
}

/**
  Calculate harmonic score and partial derivative of the score
  with respect to the given feature if the feature is less than
  the mean. Otherwise, the score is zero.

  \param[in] feature Value of feature being tested.

  \param[in] mean Expected mean value of the feature.

  \param[in] sd Allowable standard deviation value of the feature.

  \param[out] deriv Partial derivative of the score with respect to
                      the feature value.
 */

Float HarmonicLowerBound::operator()(Float feature, Float& deriv)
{
  if (feature >= mean_) {
    deriv = 0.0;
    return 0.0;
  } else
    return harmonic(feature, deriv);
}

/** Destructor */
HarmonicUpperBound::~HarmonicUpperBound()
{
  IMP_LOG(VERBOSE, "Delete HarmonicUpperBound: beware of early Python "
	  << "calls to destructor.");
}

/**
  Calculate harmonic score with respect to the given feature if
  the feature is greater than the mean. Otherwise, the score is zero.

  \param[in] feature Value of feature being tested.

  \param[in] mean Expected mean value of the feature.

  \param[in] sd Allowable standard deviation value of the feature.
 */

Float HarmonicUpperBound::operator()(Float feature)
{
  if (feature <= mean_)
    return 0.0;
  else
    return harmonic(feature);
}

/**
  Calculate harmonic score and partial derivative of the score
  with respect to the given feature if the feature is greater than
  the mean. Otherwise, the score is zero.

  \param[in] feature Value of feature being tested.

  \param[in] mean Expected mean value of the feature.

  \param[in] sd Allowable standard deviation value of the feature.

  \param[out] deriv Partial derivative of the score with respect to
                      the feature value.
 */

Float HarmonicUpperBound::operator()(Float feature, Float& deriv)
{
  if (feature <= mean_) {
    deriv = 0.0;
    return 0.0;
  } else
    return harmonic(feature, deriv);
}

//######### ScoreFuncParams #########

/**
  Constructor
 */

BasicScoreFuncParams::BasicScoreFuncParams(std::string score_func_type, 
                                                 Float mean, Float sd)
{
  mean_ = mean;
  sd_ = sd;
  score_func_type_ = score_func_type;
}


/**
Destructor
 */

BasicScoreFuncParams::~BasicScoreFuncParams ()
{
}


/**
Set the mean to use in calculating this score function.

\param[in] mean Value for the mean.
*/

void BasicScoreFuncParams::set_mean(Float mean)
{
  mean_ = mean;
}

/**
Set the standard deviation to use in calculating this score function.

\param[in] sd Value for the standard deviation.
*/

void BasicScoreFuncParams::set_sd(Float sd)
{
  sd_ = sd;
}

/**
Set the type of score function to use.

\param[in] mean Value for the mean.
*/

void BasicScoreFuncParams::set_score_func_type(std::string score_func_type)
{
  score_func_type_ = score_func_type;
}


/**
Create the specified score function with the current set of parameters
and return a pointer to that score function.

\return pointer to score function.
*/

ScoreFunc* BasicScoreFuncParams::create_score_func(void)
{
  if (score_func_type_ == "harmonic") {
    return new Harmonic(mean_, sd_);
  } else if (score_func_type_ == "harmonic_lower_bound") {
    return new HarmonicLowerBound(mean_, sd_);
  } else if (score_func_type_ == "harmonic_upper_bound") {
    return new HarmonicUpperBound(mean_, sd_);
  }
  
  IMP_check(0, "Unknown score function: " << score_func_type_,
	    std::out_of_range("Unknown score function"));
  return NULL;
}



}  // namespace imp
