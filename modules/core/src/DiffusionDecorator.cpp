/**
 *  \file DiffusionDecorator.cpp   \brief Simple xyzr decorator.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/core/DiffusionDecorator.h"
#include <IMP/algebra/Vector3D.h>

#ifdef IMP_USE_CGAL
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Min_sphere_of_spheres_d.h>
#endif

IMPCORE_BEGIN_NAMESPACE

typedef
unit::Shift<unit::Multiply<unit::Pascal,
                           unit::Second>::type,
            -3>::type MillipascalSecond;

static MillipascalSecond eta(unit::Kelvin T)
{
  const std::pair<unit::Kelvin, MillipascalSecond> points[]
    ={ std::make_pair(unit::Kelvin(273+10.0),
                      MillipascalSecond(1.308)),
       std::make_pair(unit::Kelvin(273+20.0),
                      MillipascalSecond(1.003)),
       std::make_pair(unit::Kelvin(273+30.0),
                      MillipascalSecond(0.7978)),
       std::make_pair(unit::Kelvin(273+40.0),
                      MillipascalSecond(0.6531)),
       std::make_pair(unit::Kelvin(273+50.0),
                      MillipascalSecond(0.5471)),
       std::make_pair(unit::Kelvin(273+60.0),
                      MillipascalSecond(0.4668)),
       std::make_pair(unit::Kelvin(273+70.0),
                      MillipascalSecond(0.4044)),
       std::make_pair(unit::Kelvin(273+80.0),
                      MillipascalSecond(0.3550)),
       std::make_pair(unit::Kelvin(273+90.0),
                      MillipascalSecond(0.3150)),
       std::make_pair(unit::Kelvin(273+100.0),
                      MillipascalSecond(0.2822))};

  const unsigned int npoints= sizeof(points)/sizeof(std::pair<float,float>);
  if (T < points[0].first) {
    return points[0].second;
  } else {
    for (unsigned int i=1; i< npoints; ++i) {
      if (points[i].first > T) {
        float f= ((T - points[i-1].first)
                  /(points[i].first - points[i-1].first))
          .get_normalized_value();
        MillipascalSecond ret=
          (1.0-f) *points[i-1].second + f*points[i].second;
        return ret;
      }
    }
  }
  return points[npoints-1].second;
}

unit::Femtojoule kt(unit::Kelvin t) {
  return IMP::unit::Femtojoule(IMP::internal::KB*t);
}



FloatKey DiffusionDecorator::get_D_key() {
  static FloatKey k("D");
  return k;
}
void DiffusionDecorator::set_D_from_radius_in_angstroms(Float ir) {
  return set_D_from_radius_in_angstroms(ir,
                            IMP::internal::DEFAULT_TEMPERATURE.get_value());
}

void DiffusionDecorator::set_D_from_radius_in_angstroms(Float ir,
                                                        Float it) {
  unit::Kelvin t(it);
  unit::Angstrom r(ir);
  MillipascalSecond e=eta(t);
  //unit::MKSUnit<-13, 0, 1, 0, -1> etar( e*r);
  /*std::cout << e << " " << etar << " " << kt << std::endl;
  std::cout << "scalar etar " << (unit::Scalar(6*unit::PI)*etar)
            << std::endl;
  std::cout << "ret pre conv " << (kt/(unit::Scalar(6* unit::PI)*etar))
  << std::endl;*/
  unit::SquareCentimeterPerSecond ret(kt(t)/(6.0* IMP::internal::PI*e*r));
  //std::cout << "ret " << ret << std::endl;
  set_D_in_cm2_per_second(ret.get_value());
}

void DiffusionDecorator::show(std::ostream &out, std::string prefix) const
{
  XYZDecorator::show(out, prefix);
  out << "D= " << get_D_in_cm2_per_second() << "cm^2/sec";

}

IMPCORE_END_NAMESPACE
