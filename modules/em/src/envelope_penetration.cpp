/**
 *  \file envelope_penetration.cpp
 *  \brief functions for calculation envelope penetration
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */
#include <IMP/em/envelope_penetration.h>
#include <IMP/core/XYZ.h>
IMPEM_BEGIN_NAMESPACE

long get_number_of_particles_outside_of_the_density(DensityMap *dmap,
                                                    const Particles &ps) {
  long out_of_dens=0;
  core::XYZsTemp xyz = core::XYZsTemp(ps);
  for(int i=0;i<ps.size();i++) {
    if (!dmap->is_part_of_volume(xyz[i].get_coordinates())) {
      out_of_dens++;
    }
  }
  return out_of_dens;
}
IMPEM_END_NAMESPACE
