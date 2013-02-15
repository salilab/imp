/**
    *  \file estimates.cpp
    *  \brief Estimates of various physical quantities.
    *
    *  Copyright 2007-2013 IMP Inventors. All rights reserved.
    */

#include "IMP/atom/estimates.h"
#include <IMP/atom/Simulator.h>
#include <IMP/algebra/Rotation3D.h>
#include <IMP/internal/units.h>
#include <IMP/constants.h>
#include <IMP/internal/constants.h>

IMPATOM_BEGIN_NAMESPACE


double get_protein_density_from_reference(
      ProteinDensityReference density_reference){
  double density=0.625; // ALBER reference
  switch (density_reference) {
   // Alber et al. (2005) r = 0.726*std::pow(m, .3333);
    case ALBER:
        break;
   // Harpaz et al. 0.826446=1/1.21 Da/A3 ~ 1.372 g/cm3
    case HARPAZ:
      density = 0.826446;
      break;
    // Andersson and Hovmoller (1998) Theoretical 1.22 g/cm3
    case ANDERSSON :
      density = 0.7347;
      break;
    // Tsai et al. (1999) Theoretical 1.40 g/cm3
    case TSAI:
      density = 0.84309;
      break;
    // Quillin and Matthews (2000) Theoretical 1.43 g/cm3
    case QUILLIN:
      density = 0.86116;
      break;
    // Squire and Himmel (1979) and Gekko and Noguchi (1979) Experimental 1.37
    case SQUIRE:
      density = 0.82503;
      break;
    // unknown reference;
    default :
    IMP_WARN("unknown density reference... Density set to its default value.");
   }
  return density;
 }

double get_volume_from_mass(double m,ProteinDensityReference ref) {
  return m/get_protein_density_from_reference(ref);
}

double get_mass_from_volume(double m,ProteinDensityReference ref) {
  return m*get_protein_density_from_reference(ref);
}

double get_mass_from_number_of_residues(unsigned int num_aa) {
  return 110.0*num_aa;
}

double get_volume_from_residue_type(ResidueType rt) {
  typedef std::pair<ResidueType, double> RP;
  static const RP radii[]={RP(ResidueType("ALA"), 2.516),
                     RP(ResidueType("ARG"), 3.244),
                     RP(ResidueType("ASN"), 2.887),
                     RP(ResidueType("ASP"), 2.866),
                     RP(ResidueType("CYS"), 2.710),
                     RP(ResidueType("GLN"), 3.008),
                     RP(ResidueType("GLU"), 2.997),
                     RP(ResidueType("GLY"), 2.273),
                     RP(ResidueType("HIS"), 3.051),
                     RP(ResidueType("ILE"), 3.047),
                     RP(ResidueType("LEU"), 3.052),
                     RP(ResidueType("LYS"), 3.047),
                     RP(ResidueType("MET"), 3.068),
                     RP(ResidueType("PHE"), 3.259),
                     RP(ResidueType("PRO"), 2.780),
                     RP(ResidueType("SER"), 2.609),
                     RP(ResidueType("THR"), 2.799),
                     RP(ResidueType("TRP"), 3.456),
                     RP(ResidueType("TYR"), 3.318),
                     RP(ResidueType("VAL"), 2.888)};
  static const IMP::base::map<ResidueType, double> radii_map(radii,
                                                                 radii
                                                                 +sizeof(radii)
                                                                 /sizeof(RP));
  if (radii_map.find(rt) == radii_map.end()) {
    IMP_THROW( "Can't approximate volume of non-standard residue "
               << rt, ValueException);
  }
  double r= radii_map.find(rt)->second;
  IMP_INTERNAL_CHECK(r>0, "Read garbage r for "<< rt);
  return algebra::get_volume(algebra::Sphere3D(algebra::get_zero_vector_d<3>(),
                                               r));
}

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
                      MillipascalSecond(0.2822)),
       std::make_pair(unit::Kelvin(std::numeric_limits<Float>::max()),
                      MillipascalSecond(0.2822))};

  //const unsigned int npoints= sizeof(points)/sizeof(std::pair<float,float>);
  if (T < points[0].first) {
    return points[0].second;
  } else {
    unsigned int i;
    for (i=1; points[i].first < T; ++i) {}
    double f= ((T - points[i-1].first)
              /(points[i].first - points[i-1].first));
    MillipascalSecond ret=
      (1.0-f) *points[i-1].second + f*points[i].second;
    return ret;
  }
}

namespace {
unit::Femtojoule kt(unit::Kelvin t) {
  return IMP::unit::Femtojoule(IMP::internal::KB*t);
}
}

double get_einstein_diffusion_coefficient(double r) {
  MillipascalSecond e=eta(IMP::internal::DEFAULT_TEMPERATURE);
  unit::SquareAngstromPerFemtosecond
      ret(kt(IMP::internal::DEFAULT_TEMPERATURE)
          /(6.0* PI*e*unit::Angstrom(r)));
    return ret.get_value();
}

double get_einstein_rotational_diffusion_coefficient(double r) {
  MillipascalSecond e=eta(IMP::internal::DEFAULT_TEMPERATURE);
  //double kt= get_kt(IMP::internal::DEFAULT_TEMPERATURE);
  unit::PerFemtosecond ret=kt(IMP::internal::DEFAULT_TEMPERATURE)
      /(8*PI*e*square(unit::Angstrom(r))*unit::Angstrom(r));
  return ret.get_value();
}

double get_diffusion_length(double D, double dtfs) {
  unit::Angstrom d= sqrt(6.0*unit::SquareAngstromPerFemtosecond(D)
                         *unit::Femtosecond(dtfs));
  return strip_units(d);
}

/** Return the scale for diffusion under the specified force,
    the diffusion coefficient D and the time step t.

    @param D     diffusion coefficient
    @param force force coefficient
    @param dtfs  time step in femtoseconds
    @param temp  temperature in Kalvin
\*/
double get_diffusion_length
(double D, double force, double dtfs, double temp)
{
  unit::Divide<unit::Femtosecond,
               unit::Femtojoule>::type dtikt
    =unit::Femtosecond(dtfs)
    /unit::Femtojoule(IMP::internal::KB*unit::Kelvin(temp));
  unit::Femtonewton nforce(get_force_in_femto_newtons(force));
  //unit::Angstrom R(sampler_());
  unit::Angstrom force_term(nforce*
                            unit::SquareAngstromPerFemtosecond(D)
                            *dtikt);
  return strip_units(force_term);
}


double get_diffusion_angle(double D, double dtfs) {
  double a= sqrt(6.0*unit::PerFemtosecond(D)
                   *unit::Femtosecond(dtfs));
  return a;
}

namespace {
  template <class It>
  double get_diffusion_coefficient(It b, It e,
                                   int index,
                                   double dt) {
    double sum=0;
    for (It c= b; c != e; ++c) {
      sum+=(*c)[index];
    }
    double average= sum/std::distance(b,e);
    double stdsum=0;
    for (It c= b; c != e; ++c) {
      stdsum+=algebra::get_squared((*c)[index]-average);
    }
    double std= stdsum/std::distance(b,e);
    // unit::Angstrom d= sqrt(6.0*unit::SquareAngstromPerFemtosecond(D)
    //                        *unit::Femtosecond(dtfs));
    // d^2= 6*D*dtfs
    // D= d^2/6dt
    return std/(2.0*dt);
  }
}

double
get_diffusion_coefficient(const algebra::Vector3Ds &displacements,
                          double dt) {
  algebra::Vector3D Ds;
  for (unsigned int i=0; i< 3; ++i) {
    Ds[i]= get_diffusion_coefficient(displacements.begin(),
                                     displacements.end(), i,  dt);
  }
  IMP_LOG_TERSE( "Diffusion coefficients are " << Ds << std::endl);
  int len=displacements.size()/2;
  algebra::Vector3D Ds0;
  for (unsigned int i=0; i< 3; ++i) {
    Ds0[i]= get_diffusion_coefficient(displacements.begin(),
                                      displacements.begin()+len, i, dt);
  }
  algebra::Vector3D Ds1;
  for (unsigned int i=0; i< 3; ++i) {
    Ds1[i]= get_diffusion_coefficient(displacements.begin()+len,
                                      displacements.end(), i, dt);
  }
  IMP_LOG_TERSE( "Partial coefficients are " << Ds0 << " and "
          << Ds1 << std::endl);
  return std::accumulate(Ds1.coordinates_begin(),
                         Ds1.coordinates_end(), 0.0)/3.0;
}


double
get_rotational_diffusion_coefficient(const algebra::Rotation3Ds &displacements,
                                     double dt) {
  Floats diffs(displacements.size()-1);
  for (unsigned int i=1; i< displacements.size(); ++i) {
    algebra::Rotation3D orot=displacements[i-1];
    algebra::Rotation3D crot=displacements[i];
    algebra::Rotation3D diff=crot/orot;
    diffs[i-1]=
        algebra::get_axis_and_angle(diff).second;
  }
  double mean= std::accumulate(diffs.begin(),diffs.end(), 0.0)/diffs.size();
  double stdsum=0;
  for (unsigned int i=0; i < diffs.size(); ++i) {
    stdsum+= algebra::get_squared(diffs[i]- mean);
  }
  double sigma= stdsum/diffs.size();
  return sigma/(6.0*dt);
}

IMPATOM_END_NAMESPACE
