/**
 *  \file units.h     \brief Classes to help with converting between units.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_UNITS_H
#define IMPKERNEL_UNITS_H

#include "Unit.h"
#include "../base_types.h"


#include <sstream>
#include <cmath>
#include <limits>

IMPKERNEL_BEGIN_NAMESPACE

namespace internal
{

// want it for conversions
extern IMPKERNELEXPORT const unit::ExponentialNumber<3> JOULES_PER_KILOCALORIE;
extern IMPKERNELEXPORT const unit::ExponentialNumber<23> NA;


namespace unit
{

namespace internal
{

struct AtomsPerMol {};


struct MDEnergyTag;

template <>
inline std::string get_unit_name<MDEnergyTag>(int) {
  std::string os[]= {"Cal/Mol"};
  return os[0];
}


struct MDDerivativeTag;

template <>
inline std::string get_unit_name<MDDerivativeTag>(int) {
  std::string os[]= {"Cal/(A Mol)"};
  return os[0];
}


struct MKSTag{};

template <>
inline std::string get_unit_name<MKSTag>(int o) {
  std::string os[]= {"kg", "m", "s", "k", "Cal"};
  return os[o];
}


  struct DaltonTag{};

template <>
inline std::string get_unit_name<DaltonTag>(int) {
  std::string os[]= {"Da"};
  return os[0];
}

  struct MolarTag{};

template <>
inline std::string get_unit_name<MolarTag>(int) {
  std::string os[]= {"Mol"};
  return os[0];
}

} // namespace unit::internal

typedef boost::mpl::vector_c<int, 0,0,0,0,0> Scalar;
typedef boost::mpl::vector_c<int, 1,0,0,0,0> Mass;
typedef boost::mpl::vector_c<int, 0,1,0,0,0> Length;
typedef boost::mpl::vector_c<int, 0,3,0,0,0> Volume;
typedef boost::mpl::vector_c<int, 0,0,1,0,0> Time;
typedef boost::mpl::vector_c<int, 0,0,0,1,0> Temperature;
typedef boost::mpl::vector_c<int, 0,0,0,0,1> HeatEnergy;

typedef boost::mpl::vector_c<int, 1,2,-2,0,0> Energy;
typedef boost::mpl::vector_c<int, 1,1,-2,0,0> Force;
typedef boost::mpl::vector_c<int, 1,-1,-2,0,0> Pressure;
typedef boost::mpl::vector_c<int, 0,-1,0,0,1> HeatEnergyDerivative;


typedef Unit<internal::MKSTag, 0, Scalar> MKSScalar;
typedef Unit<internal::MKSTag, 0, Length> Meter;
typedef Unit<internal::MKSTag, -2, Length> Centimeter;
typedef Unit<internal::MKSTag, 0, Mass> Kilogram;
typedef Unit<internal::MKSTag, -3, Mass> Gram;
typedef Unit<internal::MKSTag, 0, Time> Second;
typedef Unit<internal::MKSTag, 0, Energy> Joule;
typedef Unit<internal::MKSTag, 0, Temperature> Kelvin;
typedef Unit<internal::MKSTag, -3, Volume> Liter;
typedef Unit<internal::MKSTag, 0, Volume> CubicMeter;
typedef Shift<Meter, -10>::type Angstrom;
typedef Shift<Meter, -9>::type Nanometer;
typedef Multiply<Angstrom, Angstrom>::type SquareAngstrom;
typedef Shift<Joule, -15>::type Femtojoule;
typedef Shift<Joule, -12>::type Picojoule;
typedef Divide<Joule, Kelvin>::type JoulePerKelvin;
typedef Multiply<Meter, Meter>::type SquareMeter;
typedef Divide<SquareMeter, Second>::type SquareMeterPerSecond;
typedef Unit<internal::MKSTag, -15, Time> Femtosecond;
typedef Shift<SquareMeterPerSecond, -4>::type SquareCentimeterPerSecond;
typedef Divide<SquareAngstrom, Femtosecond>::type SquareAngstromPerFemtosecond;
typedef Divide<MKSScalar, Femtosecond>::type PerFemtosecond;
typedef Unit<internal::MKSTag, -15, Force> Femtonewton;
typedef Unit<internal::MKSTag, -12,Force> Piconewton;
typedef Unit<internal::MKSTag, -18, Mass> Femtogram;
typedef Divide<Piconewton, Angstrom>::type
PiconewtonPerAngstrom;
typedef Unit<internal::MKSTag, 0, Pressure> Pascal;
typedef Unit<internal::MKSTag, 0, HeatEnergy> Kilocalorie;
typedef Unit<internal::MKSTag, -21, HeatEnergy> YoctoKilocalorie;
typedef Divide<Kilocalorie, Meter>::type KilocaloriePerMeter;
typedef Divide<Kilocalorie, Angstrom>::type KilocaloriePerAngstrom;
// Calorie is ambiguous given our naming convention
typedef Shift<KilocaloriePerMeter, -21>::type YoctoKilocaloriePerMeter;
typedef Shift<KilocaloriePerAngstrom, -21>::type YoctoKilocaloriePerAngstrom;
typedef Multiply<Multiply<Centimeter, Centimeter>::type,
                      Centimeter>::type CubicCentimeter;
typedef Multiply<Multiply<Angstrom, Angstrom>::type,
                      Angstrom>::type CubicAngstrom;
typedef Multiply<Angstrom, Angstrom>::type  SquaredAngstrom;
typedef Shift<CubicCentimeter, 3>::type Liter;
typedef Divide<Gram, CubicCentimeter>::type GramPerCubicCentimeter;
typedef Shift<Meter, -6>::type Micron;


template <int EXP, class Units>
inline typename Exchange<Unit<internal::MKSTag, EXP, Units>,
        Kilocalorie, Joule, 4>::type
convert_Cal_to_J(Unit<internal::MKSTag, EXP, Units> i) {
  typedef typename Exchange<Unit<internal::MKSTag, EXP, Units>,
    Kilocalorie, Joule, 4>::type Return;
  return Return(i.get_exponential_value()
                * JOULES_PER_KILOCALORIE);
}

template <int EXP, class Units>
inline typename Exchange<Unit<internal::MKSTag, EXP, Units>,
        Joule, Kilocalorie, -3>::type
convert_J_to_Cal(Unit<internal::MKSTag, EXP, Units> i) {
  typedef typename Exchange<Unit<internal::MKSTag, EXP, Units>,
    Joule, Kilocalorie, -3>::type Return;
  return Return(i.get_exponential_value()
                / JOULES_PER_KILOCALORIE);
}


//! Marker constant to handle coversions
extern const IMPKERNELEXPORT internal::AtomsPerMol ATOMS_PER_MOL;



typedef Unit<internal::MDEnergyTag, 0, SingletonUnit> KilocaloriePerMol;
typedef Unit<internal::MDDerivativeTag, 0, SingletonUnit>
KilocaloriePerAngstromPerMol;

inline Unit<internal::MKSTag, -24, HeatEnergy >
operator/(KilocaloriePerMol k, internal::AtomsPerMol) {
  return Unit<internal::MKSTag, -24, HeatEnergy >(k.get_value()*1.661);
}

inline Unit<internal::MKSTag, -14, HeatEnergyDerivative >
operator/(KilocaloriePerAngstromPerMol k,
          internal::AtomsPerMol) {
  return Unit<internal::MKSTag, -14,
    HeatEnergyDerivative >(k.get_value()*1.661);
}

template <int EXP>
inline KilocaloriePerMol operator*(Unit<internal::MKSTag, EXP, HeatEnergy> k,
                            internal::AtomsPerMol) {
  return KilocaloriePerMol(k.get_exponential_value()
                           *NA);
}

template <int EXP>
inline KilocaloriePerAngstromPerMol
operator*(Unit<internal::MKSTag, EXP,
          HeatEnergyDerivative> k,
          internal::AtomsPerMol) {
  return KilocaloriePerAngstromPerMol(k.get_exponential_value()
                                      *NA*ExponentialNumber<-10>(1));
}

template <int EXP>
inline KilocaloriePerMol operator*(internal::AtomsPerMol,
                            Unit<internal::MKSTag, EXP,HeatEnergy> k) {
  return operator*(k, ATOMS_PER_MOL);
}



template <int EXP>
inline KilocaloriePerAngstromPerMol
operator*(internal::AtomsPerMol,
          Unit<internal::MKSTag, EXP,
          HeatEnergyDerivative >
          /*typename YoctoKilocaloriePerAngstrom::Units>*/ k) {
  return operator*(k, ATOMS_PER_MOL);
}


  // define moles
typedef Unit<internal::MolarTag, 0, SingletonUnit> Molar;
typedef Unit<internal::MolarTag, -6, SingletonUnit> Micromolar;

template <int E>
inline Molar
molarity_from_count(double count, Unit<internal::MKSTag, E, Volume> volume) {
  Unit<internal::MKSTag, -E,
    boost::mpl::vector_c<int, 0,-3,0,0,0>  >
    per_cubic_meter(unit::ExponentialNumber<0>(count)/volume);
  typedef Inverse<CubicMeter>::type PerCubicMeter;
  PerCubicMeter moles_per_cubic_meter(per_cubic_meter/NA);
  typedef Inverse<Liter>::type PerLiter;
  PerLiter moles_per_liter= moles_per_cubic_meter;
  return Molar(moles_per_liter.get_value());
}


template <int E1>
inline Unit<internal::MKSTag,E1+23-3,boost::mpl::vector_c<int, 0,-3,0,0,0>  >
density_from_molarity(Unit<internal::MolarTag, E1, SingletonUnit> molar) {
  Unit<internal::MKSTag,E1+23, boost::mpl::vector_c<int, 0,-3,0,0,0>  >
    count_per_liter(molar.get_exponential_value()*NA);
  return count_per_liter*1000.0;
}

// define Daltons

typedef Unit<internal::DaltonTag, 0, SingletonUnit> Dalton;
typedef Unit<internal::DaltonTag, 3, SingletonUnit> Kilodalton;

template <int EXP>
inline Unit<internal::MKSTag, EXP-26, Mass>
convert_to_mks(Unit<internal::DaltonTag, EXP, SingletonUnit> d) {
  return Unit<internal::MKSTag, EXP-26, Mass>(d.get_value()/NA.get_value());
}

template <int EXP>
inline Unit<internal::DaltonTag, EXP+26, SingletonUnit>
convert_to_Dalton(Unit<internal::MKSTag, EXP, Mass> d) {
  return Unit<internal::DaltonTag,
    EXP+26, SingletonUnit>(d.get_value()*NA.get_value());
}

template <class TagT, int EXPT, class UnitsT>
inline double strip_units(Unit<TagT, EXPT, UnitsT> u) {
  return u.get_value();
}


} // namespace unit

} // namespace internal

namespace unit= internal::unit;

IMPKERNEL_END_NAMESPACE

#endif  /* IMPKERNEL_UNITS_H */
