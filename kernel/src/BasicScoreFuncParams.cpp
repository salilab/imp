/**
 *  \file BasicScoreFuncParams.cpp  \brief Factory for creating UnaryFunctors.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/ScoreFuncParams.h"
#include "IMP/unary_functors/Harmonic.h"
#include "IMP/unary_functors/HarmonicLowerBound.h"
#include "IMP/unary_functors/HarmonicUpperBound.h"
#include "IMP/log.h"
#include "mystdexcept.h"

namespace IMP
{

//! Constructor
BasicScoreFuncParams::BasicScoreFuncParams(std::string score_func_type,
                                           Float mean, Float sd)
{
  mean_ = mean;
  sd_ = sd;
  score_func_type_ = score_func_type;
}


//! Destructor
BasicScoreFuncParams::~BasicScoreFuncParams()
{
}


//! Set the mean to use in calculating this score function.
/** \param[in] mean Value for the mean.
 */
void BasicScoreFuncParams::set_mean(Float mean)
{
  mean_ = mean;
}

//! Set the standard deviation to use in calculating this score function.
/** \param[in] sd Value for the standard deviation.
 */
void BasicScoreFuncParams::set_sd(Float sd)
{
  sd_ = sd;
}

//! Set the type of score function to use.
/** \param[in] score_func_type Name of the score function type
 */
void BasicScoreFuncParams::set_score_func_type(std::string score_func_type)
{
  score_func_type_ = score_func_type;
}


//! Create a new score function with the current set of parameters.
/** \return pointer to score function.
 */
UnaryFunctor* BasicScoreFuncParams::create_score_func()
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

}  // namespace IMP
