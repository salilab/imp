/*
 *  Score_Func.h
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 * Used to be Math_Form but Daniel thought that name was too vague.
 */

#ifndef __IMP_SCORE_FUNC_H
#define __IMP_SCORE_FUNC_H

#include "IMP_config.h"
#include "Base_Types.h"

namespace imp
{

// Abstract functor class for score functions
// Decided to pass the derivative by reference so that the functor form could be
// maintained for both the score calls and the score and partial derivative calls.
class IMPDLLEXPORT Score_Func
{
public:
  Score_Func() {}
  virtual ~Score_Func() {}
  virtual Float operator()(Float feature, Float mean, Float sd) = 0;
  virtual Float operator()(Float feature, Float mean, Float sd, Float& deriv) = 0;
};

// Harmonic score function (symmetric about the mean)
class IMPDLLEXPORT Harmonic : public Score_Func
{
public:
  Harmonic() {}
  virtual ~Harmonic() {}

  virtual Float operator()(Float feature, Float mean, Float sd, Float& deriv);
  virtual Float operator()(Float feature, Float mean, Float sd);

  Float harmonic(Float feature, Float mean, Float sd, Float& deriv);
  Float harmonic(Float feature, Float mean, Float sd);
};

// Lower bound harmonic score function (non-zero when feature < mean)
class IMPDLLEXPORT Harmonic_Lower_Bound : public Harmonic
{
public:
  Harmonic_Lower_Bound() {}
  virtual ~Harmonic_Lower_Bound() {}

  virtual Float operator()(Float feature, Float mean, Float sd, Float& deriv);
  virtual Float operator()(Float feature, Float mean, Float sd);
};

// Upper bound harmonic score function (non-zero when feature > mean)
class IMPDLLEXPORT Harmonic_Upper_Bound : public Harmonic
{
public:
  Harmonic_Upper_Bound() {}
  virtual ~Harmonic_Upper_Bound() {}

  virtual Float operator()(Float feature, Float mean, Float sd, Float& deriv);
  virtual Float operator()(Float feature, Float mean, Float sd);
};

} // namespace imp

#endif  /* __IMP_SCORE_FUNC_H */
