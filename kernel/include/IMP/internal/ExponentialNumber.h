/**
 *  \file ExponentialNumber.h
 *  \brief Classes to add compile time exponents to numbers.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_EXPONENTIAL_NUMBER_H
#define __IMP_EXPONENTIAL_NUMBER_H

#include "../macros.h"

#include <iostream>
#include <cmath>
#include <iomanip>
#include <limits>

namespace IMP
{

namespace internal
{

namespace unit
{

/** This helper class implements a floating point number
    with an extra exponent. The actual value is the number 
    times 10^EXP. I needed this since I was running out of bits
    in the exponent when converting between vastly different units.
    It also means that you can have a type for both nanometer
    and meter without much pain. Since the exponent is stored
    as a template argument, there shouldn't be any runtime overhead.
 */
template <int EXP>
class ExponentialNumber
{
  double v_;

  typedef ExponentialNumber<EXP> This;

  template <int OEXP>
  void copy_from(ExponentialNumber<OEXP> o) {
    const int diff = OEXP-EXP;
    v_=o.v_;
    float factor= std::pow(10.0, static_cast<double>(diff));
    v_*=factor;
  }
  bool is_default() const {
    return v_== std::numeric_limits<double>::infinity();
  }

  template <int E>
  friend class ExponentialNumber;
public:


  ExponentialNumber(): v_(std::numeric_limits<double>::infinity()) {}
  template <int OEXP>
  ExponentialNumber(ExponentialNumber<OEXP> o) {
    copy_from(o);
  }
  explicit ExponentialNumber(double d): v_(d) {
  }
  template <int OEXP>
  This &operator=(ExponentialNumber<OEXP> o) {
    copy_from(o);
    return *this;
  }

  //! Compute the value with the exponent
  double get_normalized_value() const {
    double ret=v_;
    return ret*std::pow(10.0, EXP);
  }
  //! Get the stored value ignoring the exponent
  double get_value() const {
    return v_;
  }
  template <int OEXP>
  ExponentialNumber<EXP> operator+(ExponentialNumber<OEXP> o) const {
    return This(v_+ This(o).v_);
  }
  template <int OEXP>
  ExponentialNumber<EXP> operator-(ExponentialNumber<OEXP> o) const {
    return This(v_- This(o).v_);
  }

  template <int OEXP>
  ExponentialNumber<EXP-OEXP> operator/(ExponentialNumber<OEXP> o) const {
    return ExponentialNumber<EXP-OEXP>(v_/o.v_);
  }

  template <int OEXP>
  ExponentialNumber<EXP+OEXP> operator*(ExponentialNumber<OEXP> o) const {
    return ExponentialNumber<OEXP+EXP>(v_*o.v_);
  }

  This operator-() const {
    return This(-v_);
  }

  void show(std::ostream &out) const {
    std::ios::fmtflags of = out.flags();
    out << setiosflags(std::ios::fixed) << v_;
    if (EXP != 0) out << "e" << EXP;
    out.flags(of);
    /** \todo I should restore the io flags.
     */
  }

  IMP_COMPARISONS_1(v_);
};

template <int E>
std::ostream &operator<<(std::ostream &out, ExponentialNumber<E> o)
{
  o.show(out);
  return out;
}

} // namespace unit

} // namespace internal

} // namespace IMP

#endif  /* __IMP_EXPONENTIAL_NUMBER_H */
