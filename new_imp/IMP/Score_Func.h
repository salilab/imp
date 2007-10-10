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
  virtual Float operator()(Float feature, Float& deriv) = 0;
  virtual Float operator()(Float feature) = 0;
};

// Harmonic score function (symmetric about the mean)
class IMPDLLEXPORT Harmonic : public Score_Func
{
public:
  Harmonic(Float mean, Float sd) {mean_ = mean; sd_ = sd;}
  virtual ~Harmonic();

  virtual Float operator()(Float feature, Float& deriv);
  virtual Float operator()(Float feature);

  Float harmonic(Float feature, Float& deriv);
  Float harmonic(Float feature);
  
protected:
  Float mean_;
  Float sd_;
};

// Lower bound harmonic score function (non-zero when feature < mean)
class IMPDLLEXPORT Harmonic_Lower_Bound : public Harmonic
{
public:
  Harmonic_Lower_Bound(Float mean, Float sd) : Harmonic(mean, sd) {}
  virtual ~Harmonic_Lower_Bound();

  virtual Float operator()(Float feature, Float& deriv);
  virtual Float operator()(Float feature);
};

// Upper bound harmonic score function (non-zero when feature > mean)
class IMPDLLEXPORT Harmonic_Upper_Bound : public Harmonic
{
public:
  Harmonic_Upper_Bound(Float mean, Float sd) : Harmonic(mean, sd) {}
  virtual ~Harmonic_Upper_Bound();

  virtual Float operator()(Float feature, Float& deriv);
  virtual Float operator()(Float feature);
};


// Abstract class containing parameters for creating Score_Func instances.
// Also has factory method for creating those instances.
class IMPDLLEXPORT Score_Func_Params
{
public:
  Score_Func_Params() {}
  virtual ~Score_Func_Params() {}
  virtual Score_Func* create_score_func(void) = 0;
};

// Basic class containing parameters for creating Score_Func instances
// that require a mean and standard deviation.
class IMPDLLEXPORT Basic_Score_Func_Params : public Score_Func_Params
{
public:
  Basic_Score_Func_Params(std::string score_func_type = "harmonic", 
                          Float mean = 1.0,
                          Float sd = 0.1);
  virtual ~Basic_Score_Func_Params();
  virtual Score_Func* create_score_func(void);
  void set_mean(Float mean);
  void set_sd(Float sd);
  void set_score_func_type(std::string score_func_type);

protected:
  Float mean_;
  Float sd_;
  std::string score_func_type_;
};


} // namespace imp

#endif  /* __IMP_SCORE_FUNC_H */
