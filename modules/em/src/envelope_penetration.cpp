/**
 *  \file envelope_penetration.cpp
 *  \brief functions for calculation envelope penetration
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/em/envelope_penetration.h>
#include <IMP/core/XYZ.h>
IMPEM_BEGIN_NAMESPACE

long get_number_of_particles_outside_of_the_density(DensityMap *dmap,
     const Particles &ps,const IMP::algebra::Transformation3D &t,float thr) {
  IMP_LOG(IMP::VERBOSE,"start calculating the how many particles out of "
         <<ps.size()<<" in density"<< std::endl);
  long out_of_dens=0;
  core::XYZsTemp xyz = core::XYZsTemp(ps);
  for (unsigned int i = 0; i < ps.size(); ++i) {
    IMP::algebra::Vector3D vec = t.get_transformed(xyz[i].get_coordinates());
    if (!dmap->is_part_of_volume(vec)) {
      out_of_dens++;
    }
    else {
      if (dmap->get_value(vec)<thr) {
        out_of_dens++;
      }
    }
  }
  IMP_LOG(IMP::VERBOSE,"the number of particles outside of the density is:"
          << out_of_dens << std::endl);
  return out_of_dens;
}
IMPEM_END_NAMESPACE
