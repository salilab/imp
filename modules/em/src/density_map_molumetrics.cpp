/**
 *  \file DensityMapVolumetics.cpp
 *  \brief Volumetric considerations in Cryo EM maps
 *
 */

#include <IMP/em/densityMapVolumetrics.h>

//#include <boost/algorithm/string/predicate.hpp>
//#include <climits>

IMPEM_BEGIN_NAMESPACE

// The default protein density (Harpaz and al 1994) 1/1.21
static double IMP_DEFAULT_PROTEIN_DENSITY=0.826446;
// Internal variable, stores the protein density used for the application
static double protein_density_ = IMP_DEFAULT_PROTEIN_DENSITY;

double get_used_protein_density(){
  return protein_density_;
 };

double set_used_protein_density(double density_value){
  IMP_USAGE_CHECK(density_value > 0,
        "A protein density value should most probably be a positive value");
  protein_density_=density_value;
  return protein_density_;
 }

double get_reference_protein_density(
      ProteinDensityReference density_reference){
  double density=0.0;
  switch (density_reference) {
   // Harpaz et all 0.826446=1/1.21 Da/A3 ~ 1.372 g/cm3
    case HARPAZ:
      density = IMP_DEFAULT_PROTEIN_DENSITY;
      break;
    // Andersson and Hovmšller (1998) Theoretical 1.22 g/cm3
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
    IMP_WARN(
    "unknown density reference... Density set to its default value.");
      density = IMP_DEFAULT_PROTEIN_DENSITY;
   }
  return density;
 }

double set_used_protein_density(
      ProteinDensityReference density_reference){
   double density = get_reference_protein_density(density_reference);
   density        = set_used_protein_density(density);
   return density;
}


Float get_volume_at_threshold(
      DensityMap* d,
      Float threshold) {
  long counter=0;//number of voxels above the threshold
  for(long l=0;l<d->get_number_of_voxels();l++) {
    if (d->get_value(l) > threshold) {
      ++counter;
     }
   }
  Float s=d->get_spacing();
  return  s*s*s*counter;
 }

Float get_molecular_mass_at_threshold(
      DensityMap* d,
      Float threshold) {
  long counter=0;//number of voxels above the threshold
  for(long l=0;l<d->get_number_of_voxels();l++) {
    if (d->get_value(l) > threshold) {
      ++counter;
     }
   }
  Float s=d->get_spacing();
  return  s*s*s*counter*get_used_protein_density();
 }

Float get_threshold_for_approximate_volume(
      DensityMap* d,
      Float desired_volume) {
  Float voxelVolume     = d->get_spacing()*d->get_spacing()*d->get_spacing();
  long numVoxelsNeeded  = desired_volume / voxelVolume;
  long mapSizeInVoxels  = d->get_number_of_voxels();
  std::vector<emreal> data(mapSizeInVoxels);
  // This costly loop could probably be replaced by a memcopy
  // provided I had access to d->data_
  for (long l=0;l<mapSizeInVoxels;++l) {
    data[l]=(d->get_value(l));
   }
  std::sort(data.begin(),data.end());
  emreal threshold = data[mapSizeInVoxels-numVoxelsNeeded];
  return static_cast<Float>(threshold);
 }

Float get_threshold_for_approximate_mass(
      DensityMap* d,
      Float desired_mass){
  Float desiredVolume = desired_mass / get_used_protein_density();
  return get_threshold_for_approximate_volume(d,desiredVolume);
}


IMPEM_END_NAMESPACE
