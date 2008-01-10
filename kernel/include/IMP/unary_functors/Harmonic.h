/**
 *  \file Harmonic.h    \brief Harmonic functor.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 */

#ifndef __IMP_HARMONIC_H
#define __IMP_HARMONIC_H

#include "../UnaryFunctor.h"

namespace IMP
{

//! Harmonic functor (symmetric about the mean)
class IMPDLLEXPORT Harmonic : public UnaryFunctor
{
public:
  Harmonic(Float mean, Float sd) {
    mean_ = mean;
    sd_ = sd;
  }
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

  void show(std::ostream &out=std::cout) const {
    out << "Harmonic: " << mean_ << " and " << sd_ << std::endl;
  }

protected:
  Float mean_;
  Float sd_;
};

} // namespace IMP

#endif  /* __IMP_HARMONIC_H */
