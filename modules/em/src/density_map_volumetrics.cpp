/**
    *  \file DensityMapVolumetics.cpp
    *  \brief Volumetric considerations in Cryo EM maps
    *
    */

#include <IMP/em/density_map_volumetrics.h>

IMPEM_BEGIN_NAMESPACE

Float get_volume_at_threshold(DensityMap* d, Float threshold) {
  long counter = 0;  // number of voxels above the threshold
  for (long l = 0; l < d->get_number_of_voxels(); l++) {
    if (d->get_value(l) > threshold) {
      ++counter;
    }
  }
  Float s = d->get_spacing();
  return s * s * s * counter;
}

Float get_molecular_mass_at_threshold(DensityMap* d, Float threshold,
                                      atom::ProteinDensityReference ref) {
  return atom::get_mass_from_volume(get_volume_at_threshold(d, threshold), ref);
}

Float get_threshold_for_approximate_volume(DensityMap* d,
                                           Float desired_volume) {
  Float voxelVolume = d->get_spacing() * d->get_spacing() * d->get_spacing();
  long numVoxelsNeeded = desired_volume / voxelVolume;
  long mapSizeInVoxels = d->get_number_of_voxels();
  std::vector<emreal> data(mapSizeInVoxels);
  // This costly loop could probably be replaced by a memcopy
  // provided I had access to d->data_
  for (long l = 0; l < mapSizeInVoxels; ++l) {
    data[l] = (d->get_value(l));
  }
  std::sort(data.begin(), data.end());
  emreal threshold = data[mapSizeInVoxels - numVoxelsNeeded];
  return static_cast<Float>(threshold);
}

Float get_threshold_for_approximate_mass(DensityMap* d, Float desired_mass,
                                         atom::ProteinDensityReference ref) {
  Float desiredVolume = atom::get_volume_from_mass(desired_mass, ref);
  return get_threshold_for_approximate_volume(d, desiredVolume);
}

IMPEM_END_NAMESPACE
