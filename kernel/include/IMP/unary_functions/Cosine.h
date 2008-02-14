/**
 *  \file Cosine.h    \brief Cosine function.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_COSINE_H
#define __IMP_COSINE_H

#include "../UnaryFunction.h"

namespace IMP
{

//! Cosine function.
/** This evaluates the function
    |k| - k cos(nf + a)
    where k is a force constant, n the periodicity, a the phase, and f the
    input value. This is most commonly used for dihedral angle restraints,
    e.g. in the CHARMM force field.

    \ingroup unaryf
 */
class IMPDLLEXPORT Cosine : public UnaryFunction
{
public:
  //! Constructor.
  /** \param[in] force_constant Force constant (score units)
      \param[in] periodicity Periodicity (generally 1-6)
      \param[in] phase Phase (radians)
   */
  Cosine(Float force_constant, int periodicity, Float phase) :
      force_constant_(force_constant), periodicity_(periodicity),
      phase_(phase) {}

  virtual ~Cosine() {}

  //! Calculate score with respect to the given feature.
  /** \param[in] feature Value of feature being tested.
      \return Score
   */
  virtual Float operator()(Float feature);

  //! Calculate score and derivative with respect to the given feature.
  /** \param[in] feature Value of feature being tested.
      \param[out] deriv Partial derivative of the score with respect to
                        the feature value.
      \return Score
   */
  virtual Float operator()(Float feature, Float& deriv);

  void show(std::ostream &out=std::cout) const {
    out << "Cosine function with force " << force_constant_
        << ", periodicity " << periodicity_ << ", and phase "
        << phase_ << std::endl;
  }

protected:
  Float force_constant_;
  int periodicity_;
  Float phase_;
};

} // namespace IMP

#endif  /* __IMP_COSINE_H */
