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
/** This is a simple score modeling an harmonic oscillator. The score is
    0.5 * k * x * x, where k is the 'force constant' and x is the distance
    from the mean.
    \ingroup unaryf
 */
class IMPDLLEXPORT Harmonic : public UnaryFunction
{
public:
  Harmonic(Float mean, Float k) : mean_(mean), k_(k) {}

  virtual ~Harmonic() {}

  //! \return the mean of this function
  Float get_mean() const {
    return mean_;
  }

  //! \return the spring constant
  Float get_k() const {
    return k_;
  }

  //! Set the mean of this function
  void set_mean(Float mean) {
    mean_ = mean;
  }

  //! Set the spring constant
  void set_k(Float k) {
    k_ = k;
  }

  //! Calculate harmonic score with respect to the given feature.
  /** \param[in] feature Value of feature being tested.
      \return Score
   */
  virtual Float operator()(Float feature) {
    Float d;
    return operator()(feature, d);
  }

  //! Calculate harmonic score and derivative with respect to the given feature.
  /** \param[in] feature Value of feature being tested.
      \param[out] deriv Partial derivative of the score with respect to
                        the feature value.
      \return Score
   */
  virtual Float operator()(Float feature, Float& deriv) {
    Float e = (feature - mean_);
    deriv = k_ * e;
    return 0.5 * k_ * e * e;
  }

  void show(std::ostream &out=std::cout) const {
    out << "Harmonic: " << mean_ << " and " << k_ << std::endl;
  }

  //! Return the k to use for a given Gaussian standard deviation.
  /** Given the standard deviation of a Gaussian distribution, get
      the force constant of the harmonic score function that yields that
      same distribution. For temperature in Kelvin, this assumes the score
      function is energy in kcal/mol, and thus returns a force constant in
      kcal/mol/A/A.
      \param[in] sd  Gaussian standard deviation, in angstroms
      \param[in] t   System temperature, in Kelvin
      \return        Force constant
   */
  static Float k_from_standard_deviation(Float sd, Float t=297.15) {
    // Gas constant in kcal/mol K
    const static Float R = 8.31441 / 4186.8;
    return R * t / square(sd);
  }

private:
  Float mean_;
  Float k_;
};

} // namespace IMP

#endif  /* __IMP_HARMONIC_H */
