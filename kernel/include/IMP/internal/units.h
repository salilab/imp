/**
 *  \file units.h     \brief Classes to help with converting between units.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_UNITS_H
#define __IMP_UNITS_H

#include "../utility.h"

#include <sstream>
#include <cmath>
#include <iomanip>
#include <limits>

namespace IMP
{

namespace internal
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
  typedef ExponentialNumber<EXP> This;

  template <int OEXP>
  void copy_from(ExponentialNumber<OEXP> o) {
    const int diff = OEXP-EXP;
    v_=o.v_;
    float factor= std::pow(10.0, static_cast<double>(diff));
    v_*=factor;
    IMP_assert(std::abs(get_normalized_value() -o.get_normalized_value())
               <= .09 * std::abs(std::max(get_normalized_value(),
                                          o.get_normalized_value())),
               "Mismatch after scaling");
  }
  bool is_default() const {
    return v_== std::numeric_limits<double>::infinity();
  }
  // should be private
  double v_;
  template <int E>
  friend class ExponentialNumber;
public:


  ExponentialNumber(): v_(std::numeric_limits<double>::infinity()) {}
  template <int OEXP>
  ExponentialNumber(ExponentialNumber<OEXP> o) {
    copy_from(o);
  }
  ExponentialNumber(double d): v_(d){
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

// helper function generate unit string names
inline void handle_unit(std::ostringstream &out, std::string name, int m)
{
  if (m>0) {
    out << name;
    if (m>1) {
      out << "^" << m<< " ";
    }
  }
}


//! A class to represent a quantity in the MKS system
/** The correct units are preserved with arithmetic operations
    (and only operations with compatible units are allowed). The EXP
    parameter is there to allow representation of quantities in cm or 
    other prefixed units.

    The template parameters are
    - EXP the shift in base 10
    - M the power for meters
    - KG the power for kilograms
    - K the power for Kelvin
    - S the power for seconds
 */
template <int EXP, int M, int KG, int K, int S>
class MKSUnit
{
  typedef MKSUnit<EXP, M, KG, K, S> This;
  typedef ExponentialNumber<EXP> V;

  template <int OE, int OM, int OKG, int OK, int OS>
  friend class MKSUnit;

  // for debugging since the debugger doesn't always show
  // template arguments
  int get_exponent() const {return EXP;}
  int get_meters() const {return M;}
  int get_kilograms() const {return KG;}
  int get_kelvin() const {return K;}
  int get_seconds() const {return S;}


  V v_;

public:
  explicit MKSUnit(V v): v_(v) {}
  MKSUnit(){}
  MKSUnit(double v): v_(v) {
    if (v < -std::numeric_limits<double>::max()) {
      std::cout << get_exponent() << get_meters()
                << get_kilograms() << get_kelvin() << get_seconds();
    }
  }
  MKSUnit(float v): v_(static_cast<double>(v)) {}
  MKSUnit(int v): v_(static_cast<double>(v)) {}

  template <int OEXP>
  MKSUnit(MKSUnit<OEXP, M, KG, K, S> o): v_(o.v_) {}

  //! MKS units can be compared if their types match
  template <int OEXP>
  bool operator<(MKSUnit<OEXP, M, KG, K, S> o) const {
    return v_ < o.v_;
  }
  //! only works with matching types
  template <int OEXP>
  bool operator>(MKSUnit<OEXP, M, KG, K, S> o) const {
    return v_ > o.v_;
  }

  void show(std::ostream &out) const {
    out << v_ << get_name();
  }

  //! Get the current value
  /** \note The value returned is the value before it is multiplied
      by the appropriate power of 10. That means, 1 Angstrom returns 1,
      as does 1 Meter.
   */
  double get_value() const {
    return v_.get_value();
  }
  //! Get the value with the proper exponent
  /** 1 Angstrom returns 1e-10.
   */
  double get_normalized_value() const {
    return v_.get_normalized_value();
  }

  V get_exponential_value() const {
    return v_;
  }

  template <int OEXP, int OM, int OKG, int OK, int OS>
  MKSUnit<OEXP+EXP, OM+M, OKG+KG, OK+K, OS+S>
  operator*(MKSUnit<OEXP, OM, OKG, OK, OS> o) const {
    return MKSUnit<OEXP+EXP, OM+M, OKG+KG, OK+K, OS+S>(v_*o.v_);
  }

  template <int OEXP, int OM, int OKG, int OK, int OS>
  MKSUnit<EXP-OEXP, M-OM, KG-OKG, K-OK, S-OS>
  operator/(MKSUnit<OEXP, OM, OKG, OK, OS> o) const {
    return MKSUnit<EXP-OEXP, M-OM, KG-OKG, K-OK, S-OS>(v_/o.v_);
  }

  This operator+(This o) const {
    return This(v_+o.v_);
  }
  This operator-(This o) const {
    return This(v_-o.v_);
  }

  //! Get a string version of the name of the units
  std::string get_name() const {
    int m=M;
    int kg=KG;
    int k=K;
    int s=S;
    int j=0;

    while (kg >=1 && m > 1 && s<=-2) {
      m-=2;
      kg-=1;
      s+=2;
      j+=1;
    }
    std::ostringstream name;
    handle_unit(name, "m", m);
    handle_unit(name, "kg", kg);
    handle_unit(name, "K", k);
    handle_unit(name, "s", s);
    handle_unit(name, "J", j);

    if (m < 0 || kg < 0 || k < 0 || s < 0 || j < 0) {
      name << "/";
      handle_unit(name, "m", -m);
      handle_unit(name, "kg", -kg);
      handle_unit(name, "K", -k);
      handle_unit(name, "s", -s);
      handle_unit(name, "J", -j);
    }
    return name.str();
  }
};

template <int EXP, int M, int KG, int K, int S>
MKSUnit<EXP/2, M/2, KG/2, K/2, S/2>
sqrt(MKSUnit<EXP, M, KG, K, S> o)
{
  return MKSUnit<EXP/2, M/2, KG/2, K/2, S/2>(std::sqrt(o.get_value()));
}

template <int EXP, int M, int KG, int K, int S>
MKSUnit<EXP*2, M*2, KG*2, K*2, S*2>
square(MKSUnit<EXP, M, KG, K, S> o)
{
  return MKSUnit<EXP*2, M*2, KG*2, K*2, S*2>(IMP::square(o.get_value()));
}

typedef MKSUnit<-10, 1, 0, 0, 0> Angstrom;
typedef MKSUnit<-20, 2, 0, 0, 0> SquaredAngstrom;
typedef MKSUnit<0, 2, 1, 0, -2> Joule;
typedef MKSUnit<-15, 2, 1, 0, -2> FemtoJoule;
typedef MKSUnit<-12, 2, 1, 0, -2> PicoJoule;
typedef MKSUnit<0, 0, 0, 1, 0> Kelvin;
typedef MKSUnit<0,2,1,-1,-2> JoulePerKelvin;
typedef MKSUnit<-15, 0,0,0,1> FemtoSecond;
typedef MKSUnit<-4,2,0,0,-1> Cm2PerSecond;
typedef MKSUnit<-15,1,1,0,-2> FemtoNewton;
typedef MKSUnit<-12,1,1,0,-2> PicoNewton;
typedef MKSUnit<-2, 0, 1, 0, -2> PicoNewtonPerAngstrom;
typedef MKSUnit<0,1,0,0,0> Meter;
typedef MKSUnit<0,0,0,0,0> Scalar;

template <int EXP, int M, int KG, int K, int S>
std::ostream &operator<<(std::ostream &out, MKSUnit<EXP, M, KG, K, S> u)
{
  u.show(out);
  return out;
}


//! Represent a value with units in kcal and angstroms per mol
/** This class is just for representing energies and derivatives
    for MD-type computations and so is a bit more limited than
    the MKS unit type.
 */
template <int EXP, int KCal, int A>
class KCalPerMolUnit
{
  typedef KCalPerMolUnit<EXP, KCal, A> This;
  template <int E, int K, int AA>
  friend class KCalPerMolUnit;
  typedef ExponentialNumber<EXP> V;
  V v_;

  int get_exponent() const {return EXP;}
  int get_kilocalories() const {return KCal;}
  int get_angstroms() const {return A;}
public:
  explicit KCalPerMolUnit(ExponentialNumber<EXP> e): v_(e) {}
  KCalPerMolUnit(float f): v_(f) {
    if (0 < -std::numeric_limits<double>::max()) {
      std::cout << get_exponent() << get_angstroms()
                << get_kilocalories();
    }
  }

  template <int OEXP>
  KCalPerMolUnit(KCalPerMolUnit<OEXP, KCal, A> o):v_(o.v_) {
  }

  template <int OEXP>
  KCalPerMolUnit(MKSUnit<OEXP, 0,0,0,0> o): v_(o.v_) {}

  //! Get the value ignoring the exponent
  double get_value() const {return v_.get_value();}


  //! Get the value ignoring the exponent
  V get_exponential_value() const {return v_;}

  void show(std::ostream &out) const {
    out << v_ << get_name();
  }

  //! Get a string representing the units
  std::string get_name() const {
    int exp=EXP;
    int kcal=KCal;
    int a=A;

    std::ostringstream name;
    handle_unit(name, "kCal", kcal);
    handle_unit(name, "A", a);

    if (kcal < 0 || a < 0) {
      name << "/";
      handle_unit(name, "kCal", -kcal);
      handle_unit(name, "A", -a);
    }
    return name.str();
  }

  template <int OEXP, int OKC, int OA>
  KCalPerMolUnit<OEXP+EXP, OKC+KCal, OA+A>
  operator*(KCalPerMolUnit<OEXP, OKC, OA> o) const {
    return KCalPerMolUnit<OEXP+EXP, OKC+KCal, OA+A>(v_*o.v_);
  }

  template <int OEXP, int OKC, int OA>
  KCalPerMolUnit<EXP-OEXP, KCal-OKC, A-OA>
  operator/(KCalPerMolUnit<OEXP, OKC, OA> o) const {
    return KCalPerMolUnit<EXP-OEXP, KCal-OKC, A-OA>(v_/o.v_);
  }

  template <int OEXP>
  This operator+(KCalPerMolUnit<OEXP, KCal, A> o) const {
    return This(v_+o.v_);
  }
  template <int OEXP>
  This operator-(KCalPerMolUnit<OEXP, KCal, A> o) const {
    return This(v_-o.v_);
  }

  template <int OEXP>
  bool operator<(KCalPerMolUnit<OEXP, KCal, A> o) const {
    return v_ < o.v_;
  }
  template <int OEXP>
  bool operator>(KCalPerMolUnit<OEXP, KCal, A> o) const {
    return v_ > o.v_;
  }
};



// want it for conversions
extern IMPDLLEXPORT const ExponentialNumber<23> NA;

extern IMPDLLEXPORT const ExponentialNumber<3> JOULES_PER_KCAL;
extern IMPDLLEXPORT const ExponentialNumber<-4> KCALS_PER_JOULE;


/** \function convert_to_kcal Convert from MKS units to kcal units
    \function convert_to_mks Convert from KCal units to MKS units.

    These don't yet work for arbitrary units since it is kind of
    annoying to get right and there are not equivalents for everything.
 */
template <int EXP>
KCalPerMolUnit<EXP+19, 1, 0>
convert_to_kcal(MKSUnit<EXP, 2, 1, 0, -2> j) {
  return KCalPerMolUnit<EXP+19, 1, 0>(j.get_exponential_value()
                                      *KCALS_PER_JOULE*NA);
}

template <int EXP>
KCalPerMolUnit<EXP+9, 1, -1>
convert_to_kcal(MKSUnit<EXP, 1, 1, 0, -2> j) {
  internal::ExponentialNumber<EXP+9> en(j.get_exponential_value()
                                        *KCALS_PER_JOULE*NA
                                        *internal::ExponentialNumber<-10>(1));
  return KCalPerMolUnit<EXP+9, 1, -1>(en);
}

template <int EXP>
MKSUnit<EXP-20, 2,1,0,-2>
convert_to_mks(KCalPerMolUnit<EXP, 1, 0> o) {
  return MKSUnit<EXP-20, 2,1,0,-2>(o.get_exponential_value()
                                   *JOULES_PER_KCAL/NA);
}

template <int EXP>
MKSUnit<EXP-10, 1,1,0,-2>
convert_to_mks(KCalPerMolUnit<EXP, 1, -1> o) {
  return MKSUnit<EXP-10, 1,1,0,-2>(o.get_exponential_value()
                                   *JOULES_PER_KCAL/NA
                                   *internal::ExponentialNumber<10>(1));
}

//! The unit for MD energies
typedef KCalPerMolUnit<0, 1, 0> KCalPerMol;
//! The unit for MD-compatible derivatives
typedef KCalPerMolUnit<0, 1, -1> KCalPerAMol;


template <int EXP, int KCal, int A>
std::ostream &operator<<(std::ostream &out, KCalPerMolUnit<EXP, KCal, A> u)
{
  u.show(out);
  return out;
}

} // namespace internal

} // namespace IMP

#endif  /* __IMP_UNITS_H */
