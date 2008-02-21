/**
 *  \file Harmonic.h    \brief Harmonic function.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_HARMONIC_H
#define __IMP_HARMONIC_H

#include "../UnaryFunction.h"

namespace IMP
{

/** \ingroup restraint
    \addtogroup unaryf Unary Functions
    Functions of a single variable. These are used by scoring functions.
 */

//! Harmonic function (symmetric about the mean)
/** \ingroup unaryf
 */
class IMPDLLEXPORT Harmonic : public UnaryFunction
{
public:
  Harmonic(Float mean, Float sd) : mean_(mean), sd_(sd) {}

  virtual ~Harmonic() {}

  //! \return the mean of this function
  Float get_mean() const {
    return mean_;
  }

  //! \return the standard deviation of this function
  Float get_sd() const {
    return sd_;
  }

  //! Set the mean of this function
  void set_mean(Float mean) {
    mean_ = mean;
  }

  //! Set the standard deviation of this function
  void set_sd(Float sd) {
    sd_ = sd;
  }

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

  void show(std::ostream &out=std::cout) const {
    out << "Harmonic: " << mean_ << " and " << sd_ << std::endl;
  }

protected:
  Float mean_;
  Float sd_;
};

} // namespace IMP

#endif  /* __IMP_HARMONIC_H */
