/**
    *  \file estimates.cpp
    *  \brief Estimates of various physical quantities.
    *
    *  Copyright 2007-2011 IMP Inventors. All rights reserved.
    */

#include "IMP/atom/estimates.h"
#include <IMP/internal/units.h>

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
  static const IMP::internal::Map<ResidueType, double> radii_map(radii,
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

IMPATOM_END_NAMESPACE
