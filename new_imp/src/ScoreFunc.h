/**
 *  \file ScoreFunc.h    \brief Scoring functions.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 * \note Used to be MathForm but Daniel thought that name was too vague.
 */

#ifndef __IMP_SCORE_FUNC_H
#define __IMP_SCORE_FUNC_H

#include "IMP_config.h"
#include "Base_Types.h"

namespace IMP
{

//! Abstract functor class for score functions
/** Decided to pass the derivative by reference so that the functor form
    could be maintained for both the score calls and the score and partial
    derivative calls.
 */
class IMPDLLEXPORT ScoreFunc
{
public:
  ScoreFunc() {}
  virtual ~ScoreFunc() {}
  virtual Float operator()(Float feature, Float& deriv) = 0;
  virtual Float operator()(Float feature) = 0;
};


//! Harmonic score function (symmetric about the mean)
class IMPDLLEXPORT Harmonic : public ScoreFunc
{
public:
  Harmonic(Float mean, Float sd) {mean_ = mean; sd_ = sd;}
  virtual ~Harmonic();

  //! Calculate harmonic score with respect to the given feature.
  /** \param[in] feature Value of feature being tested.
      \return Score
   */
  virtual Float operator()(Float feature);

  //! Calculate harmonic score and derivative with respect to the given feature.
  /** \param[in] feature Value of feature being tested.
      \param[out] deriv Partial derivative of the score with respect to
                        the feature value.
      \return Score
   */
  virtual Float operator()(Float feature, Float& deriv);

  //! Calculate harmonic score with respect to the given feature.
  /** Implemented to support lower and upper bound harmonics as well.
      \param[in] feature Value of feature being tested.
      \return Score
   */
  Float harmonic(Float feature);

  //! Calculate harmonic score and derivative with respect to the given feature.
  /** Implemented to support lower and upper bound harmonics as well.
      \param[in] feature Value of feature being tested.
      \param[out] deriv Partial derivative of the score with respect to
                        the feature value.
      \return Score
   */
  Float harmonic(Float feature, Float& deriv);
  
protected:
  Float mean_;
  Float sd_;
};


//! Lower bound harmonic score function (non-zero when feature < mean)
class IMPDLLEXPORT HarmonicLowerBound : public Harmonic
{
public:
  HarmonicLowerBound(Float mean, Float sd) : Harmonic(mean, sd) {}
  virtual ~HarmonicLowerBound();

  //! Calculate lower-bound harmonic score with respect to the given feature.
  /** If the feature is greater than or equal to the mean, the score is zero.
      \param[in] feature Value of feature being tested.
      \return Score
  */
  virtual Float operator()(Float feature);

  //! Calculate lower-bound harmonic score and derivative for a feature.
  /** If the feature is greater than or equal to the mean, the score is zero.
      \param[in] feature Value of feature being tested.
      \param[out] deriv Partial derivative of the score with respect to
                        the feature value.
      \return Score
   */
  virtual Float operator()(Float feature, Float& deriv);
};


//! Upper bound harmonic score function (non-zero when feature > mean)
class IMPDLLEXPORT HarmonicUpperBound : public Harmonic
{
public:
  HarmonicUpperBound(Float mean, Float sd) : Harmonic(mean, sd) {}
  virtual ~HarmonicUpperBound();

  //! Calculate upper-bound harmonic score with respect to the given feature.
  /** If the feature is less than or equal to the mean, the score is zero.
      \param[in] feature Value of feature being tested.
      \return Score
   */
  virtual Float operator()(Float feature);

  //! Calculate upper-bound harmonic score and derivative for a feature.
  /** If the feature is less than or equal to the mean, the score is zero.
      \param[in] feature Value of feature being tested.
      \param[out] deriv Partial derivative of the score with respect to
                        the feature value.
      \return Score
   */
  virtual Float operator()(Float feature, Float& deriv);
};


//! Abstract class containing parameters for creating ScoreFunc instances.
/** Also has factory method for creating those instances.
 */
class IMPDLLEXPORT ScoreFuncParams
{
public:
  ScoreFuncParams() {}
  virtual ~ScoreFuncParams() {}
  virtual ScoreFunc* create_score_func(void) = 0;
};


//! Basic mean+standard deviation parameter class for making ScoreFuncs.
class IMPDLLEXPORT BasicScoreFuncParams : public ScoreFuncParams
{
public:
  BasicScoreFuncParams(std::string score_func_type = "harmonic", 
                       Float mean = 1.0, Float sd = 0.1);
  virtual ~BasicScoreFuncParams();

  //! Create a new score function with the current set of parameters.
  /** \return pointer to score function.
   */
  virtual ScoreFunc* create_score_func(void);

  //! Set the mean to use in calculating this score function.
  /** \param[in] mean Value for the mean.
   */
  void set_mean(Float mean);

  //! Set the standard deviation to use in calculating this score function.
  /** \param[in] sd Value for the standard deviation.
   */
  void set_sd(Float sd);

  //! Set the type of score function to use.
  /** \param[in] score_func_type Name of the score function type
   */
  void set_score_func_type(std::string score_func_type);

protected:
  Float mean_;
  Float sd_;
  std::string score_func_type_;
};


} // namespace IMP

#endif  /* __IMP_SCORE_FUNC_H */
