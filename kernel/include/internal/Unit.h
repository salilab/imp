/**
 *  \file Unit.h     \brief Classes to help with converting between units.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_UNIT_H
#define IMP_UNIT_H


#include "ExponentialNumber.h"
#include "../utility.h"

#include <boost/mpl/vector_c.hpp>
#include <boost/mpl/clear.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/equal.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/static_assert.hpp>
#include <boost/mpl/divides.hpp>
#include <boost/mpl/multiplies.hpp>
#include <boost/mpl/plus.hpp>
#include <boost/mpl/minus.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/pop_front.hpp>
#include <boost/mpl/size.hpp>

#include <iomanip>
#include <cmath>

IMP_BEGIN_NAMESPACE

namespace internal
{


namespace unit
{


namespace internal
{

template <class Tag>
std::string get_unit_name(int o) {
  bool should_not_be_compiled;
  return "";
}

// for stupid line length limits
using boost::mpl::transform;
using boost::mpl::minus;
using boost::mpl::plus;
using boost::mpl::divides;
using boost::mpl::multiplies;
using boost::mpl::int_;
using boost::mpl::vector_c;
using boost::mpl::at;
using namespace boost::mpl::placeholders;


/*
  These classes allow the Units part of a unit to be manipulated
 */
template <class UA, class UB>
struct Divide
{
  typedef typename transform<UA, UB, minus<_1, _2> >::type type;
};

template <class UA, class UB>
struct Multiply
{
  typedef typename transform<UA, UB, plus<_1, _2> >::type type;
};

template <class UA>
struct Sqrt
{
  typedef typename transform<UA, divides<_1, int_<2> > >::type type;
};


template <class UA, class UB>
struct Exponentiate
{
  typedef typename transform<UA, UB, multiplies<_1, _2> >::type type;
};

template <class InputVector, int SZ>
struct DoNormalize
{
  typedef int type;
};

/*
  The boost MPI multiply sorts of routines return a slightly different
  type that we want so we have to change it into the exact type to get
  certain things to work right.
 */
template <class InputVector>
struct DoNormalize<InputVector, 0>
{
  typedef vector_c<int> type;
};

template <class InputVector>
struct DoNormalize<InputVector, 1>
{
  typedef vector_c<int, at<InputVector, int_<0> >::type::value> type;
};

template <class InputVector>
struct DoNormalize<InputVector, 2>
{
  typedef vector_c<int,
                   at<InputVector, int_<0> >::type::value,
                   at<InputVector, int_<1> >::type::value> type;
};

template <class InputVector>
struct DoNormalize<InputVector, 3>
{
  typedef vector_c<int,
                   at<InputVector, int_<0> >::type::value,
                   at<InputVector, int_<1> >::type::value,
                   at<InputVector, int_<2> >::type::value> type;
};

template <class InputVector>
struct DoNormalize<InputVector, 4>
{
  typedef vector_c<int,
                   at<InputVector, int_<0> >::type::value,
                   at<InputVector, int_<1> >::type::value,
                   at<InputVector, int_<2> >::type::value,
                   at<InputVector, int_<3> >::type::value> type;
};

template <class InputVector>
struct DoNormalize<InputVector, 5>
{
  typedef vector_c<int,
                   at<InputVector, int_<0> >::type::value,
                   at<InputVector, int_<1> >::type::value,
                   at<InputVector, int_<2> >::type::value,
                   at<InputVector, int_<3> >::type::value,
                   at<InputVector, int_<4> >::type::value> type;
};

template <class InputVector>
struct Normalize
{
  typedef typename DoNormalize<InputVector,
                               boost::mpl::size<InputVector>::type::value >
      ::type type;
};

// Recursive type to print out the unit names and powers
template <class Tag, int O, int SZ, class Units>
struct PrintUnits
{
  PrintUnits<Tag, O+1, SZ, Units> p_;

  void operator()(std::ostream &out) const {
    std::string str= get_unit_name<Tag>(O);
    int e=  boost::mpl::at<Units, typename boost::mpl::int_<O> >::type::value;
    if (e != 0)  {
      out << " " << str;
      if (e != 1) {
        out << "^" << e;
      }
    }
    p_(out);
  }
};

//! \internal Terminate the recursion
template <class Tag, int O, class Units>
struct PrintUnits<Tag, O, O, Units >
{
  void operator()(std::ostream &out) const{};
};


//! \internal Specializaton for singleton units
template <class Tag, int O>
struct PrintUnits<Tag, O, O, boost::mpl::vector_c<int> >
{
  void operator()(std::ostream &out) const {
    std::string str= get_unit_name<Tag>(O);
    out << " " << str;
  }
};

// recursive type to check if all the unit powers are zero
template <int O, int SZ, class Units>
struct IsNoUnits
{
  typedef IsNoUnits<O+1, SZ, Units> Next;
  static const bool value= Next::value
    && !(boost::mpl::at<Units, typename boost::mpl::int_<O> >::type::value);
};


template <int O, class Units>
struct IsNoUnits<O, O, Units >
{
  static const bool value =true;
};



} // namespace unit::internal




//! \internal A base class for units
template <class TagT, int EXPT, class UnitsT>
class Unit
{
  template <class T, int E, class U>
  friend class Unit;
  typedef Unit<TagT, EXPT, UnitsT> This;
  typedef ExponentialNumber<EXPT> V;
  V v_;


  bool is_default() const {
    return v_== V();
  }
public:
  typedef TagT Tag;
  typedef UnitsT Units;
  static const int EXP= EXPT;

  explicit Unit(V v): v_(v){}
  explicit Unit(double v): v_(v){}
  //Unit(int v): v_(v){}
  Unit(){}
  template <int OEXP, class OUnits>
  Unit(Unit<Tag, OEXP, OUnits> o): v_(o.v_) {
    BOOST_STATIC_ASSERT((
                         boost::mpl::equal<Units,OUnits>::type::value
    ));
  }

  template <int OEXP>
  Unit(Unit<Tag, OEXP, Units> o): v_(o.v_) {}

  double to_scalar() const {
    BOOST_STATIC_ASSERT((internal::IsNoUnits<0,
                         boost::mpl::size<Units>::type::value, Units>::value));
    return get_normalized_value();
  }

  IMP_COMPARISONS_1(v_);

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


  template <int OEXP, class OUnits>
  Unit<Tag, OEXP+EXP,
       typename internal::Multiply<Units, OUnits>::type >
  operator*(Unit<Tag, OEXP, OUnits> o) const {
    return Unit<Tag, OEXP+EXP,
      typename internal::Multiply<Units, OUnits>::type >(v_*o.v_);
  }

  template <int OEXP, class OUnits>
  Unit<Tag, EXP-OEXP,
       typename internal::Divide<Units, OUnits>::type >
  operator/(Unit<Tag, OEXP, OUnits> o) const {
    return Unit<Tag, EXP-OEXP,
      typename internal::Divide<Units, OUnits>::type > (v_/o.v_);
  }

  This operator+(This o) const {
    return This(v_+o.v_);
  }
  This operator-(This o) const {
    return This(v_-o.v_);
  }

  This operator-() const {
    return This(-v_);
  }

  void show(std::ostream &out) const {
    out << v_;
    internal::PrintUnits<Tag, 0,
      boost::mpl::size<Units>::type::value, Units> p;
    p(out);
  }
};

template <class Tag, int EXP, class Units>
std::ostream &operator<<(std::ostream &out,
                         Unit<Tag, EXP, Units> o) {
  o.show(out);
  return out;
}


typedef boost::mpl::vector_c<int> SingletonUnit;



/** \internal need to be careful of integer division
 */
template <class Tag, int EXP, class Units>
Unit<Tag, EXP/2, typename internal::Sqrt<Units>::type >
sqrt(Unit<Tag, EXP, Units> o) {
  return Unit<Tag, EXP/2, typename internal::Sqrt<Units>::type >
    (ExponentialNumber<EXP/2>(std::sqrt(o.get_value())));
}

/** \internal
 */
template <class Tag, int EXP, class Units>
Unit<Tag, EXP*2, typename internal::Multiply<Units, Units>::type >
square(Unit<Tag, EXP, Units> o) {
  return Unit<Tag, EXP*2, typename internal::Multiply<Units, Units>::type >
    (::IMP::square(o.get_value()));
}

template <class Tag, int EXP, class Units>
Unit<Tag, EXP, Units >
operator*(Unit<Tag, EXP, Units> o, double d) {
  return Unit<Tag, EXP, Units>(d*o.get_value());
}

template <class Tag, int EXP, class Units>
Unit<Tag, EXP, Units >
operator*(double d, Unit<Tag, EXP, Units> o) {
  return Unit<Tag, EXP, Units>(d*o.get_value());
}

template <class Tag, int EXP, class Units>
Unit<Tag, EXP, Units >
operator/(Unit<Tag, EXP, Units> o, double d) {
  return Unit<Tag, EXP, Units>(o.get_value()/d);
}

template <class Tag, int EXP, class Units>
Unit<Tag, EXP, Units >
operator/(double d, Unit<Tag, EXP, Units> o) {
  return Unit<Tag, EXP, Units>(d/o.get_value());
}

// Multiply and divide Unit instantiations

template <class U0, class U1>
struct Divide
{
  BOOST_STATIC_ASSERT((boost::mpl::equal<typename U0::Tag,
                       typename U1::Tag>::type::value));
  typedef typename internal::Divide<typename U0::Units,
                                    typename U1::Units>::type raw_units;
  typedef typename internal::Normalize<raw_units>::type units;
  typedef Unit<typename U0::Tag, U0::EXP - U1::EXP, units > type;

};

template <class U0, class U1>
struct Multiply
{
  BOOST_STATIC_ASSERT((boost::mpl::equal<typename U0::Tag,
                       typename U1::Tag>::type::value));
  typedef typename internal::Multiply<typename U0::Units,
                                      typename U1::Units>::type raw_units;
  typedef typename internal::Normalize<raw_units>::type units;
  typedef Unit<typename U0::Tag, U0::EXP + U1::EXP,
                         units > type;
};

template <class U, int E>
struct Shift
{
  typedef Unit<typename U::Tag, U::EXP + E,
               typename U::Units > type;

};



template <class U, class R, class A, int DEXP>
struct Exchange
{
  BOOST_STATIC_ASSERT((boost::mpl::equal<typename U::Tag,
                      typename R::Tag>::type::value));
  BOOST_STATIC_ASSERT((boost::mpl::equal<typename U::Tag,
                      typename A::Tag>::type::value));
  typedef typename internal::Divide<typename U::Units,
                                    typename R::Units>::type Div;
  typedef typename internal::Multiply<Div, typename A::Units>::type Mul;
  typedef typename internal::Normalize<Mul>::type units;
  typedef Unit<typename U::Tag,
               U::EXP - R::EXP + A::EXP  +DEXP,
               units> type;

};

} // namespace unit

} // namespace internal

IMP_END_NAMESPACE

#endif  /* IMP_UNIT_H */
