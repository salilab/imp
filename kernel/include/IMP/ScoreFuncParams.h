/**
 *  \file ScoreFuncParams.h    \brief Factory for creating UnaryFunctions.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_SCORE_FUNC_PARAMS_H
#define __IMP_SCORE_FUNC_PARAMS_H

#include "IMP_config.h"
#include "base_types.h"
#include "UnaryFunction.h"

namespace IMP
{

//! Abstract class containing parameters for creating UnaryFunction instances.
/** Also has factory method for creating those instances.
 */
class IMPDLLEXPORT ScoreFuncParams
{
public:
  ScoreFuncParams() {}
  virtual ~ScoreFuncParams() {}
  virtual UnaryFunction* create_score_func() = 0;
};


//! Basic mean+standard deviation parameter class for making UnaryFunctions.
class IMPDLLEXPORT BasicScoreFuncParams : public ScoreFuncParams
{
public:
  BasicScoreFuncParams(std::string score_func_type = "harmonic",
                       Float mean = 1.0, Float sd = 0.1);
  virtual ~BasicScoreFuncParams();

  //! Create a new score function with the current set of parameters.
  /** \return pointer to score function.
   */
  virtual UnaryFunction* create_score_func();

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

#endif  /* __IMP_SCORE_FUNC_PARAMS_H */
