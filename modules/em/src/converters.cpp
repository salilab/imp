/**
 *  \file converters.cpp
 *  \brief impl
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
*/

#include <IMP/em/converters.h>
#include <IMP/em/Voxel.h>

IMPEM_BEGIN_NAMESPACE


void density2particles(DensityMap &dmap, Float threshold,
                              Particles &ps, Model *m) {
  Float x,y,z,val;
  Float voxel_size=dmap.get_header()->get_spacing();
  Float r=sqrt(3.*voxel_size*voxel_size);
  FloatKey dens_key = Voxel::get_density_key();
  for (long i=0;i<dmap.get_number_of_voxels();i++) {
    val = dmap.get_value(i);
    if (val > threshold) {
      Particle * p = new Particle(m);
      x = dmap.voxel2loc(i,0);
      y = dmap.voxel2loc(i,1);
      z = dmap.voxel2loc(i,2);
      Voxel::setup_particle(p,IMP::algebra::VectorD<3>(x,y,z),r,val);
      ps.push_back(p);
    }
  }
}


IMPEMEXPORT SampledDensityMap * particles2density(
   Particles &ps,
   Float resolution, Float apix,
   int sig_cuttoff,
   const FloatKey &rad_key,
   const FloatKey &weight_key)
 {
  SampledDensityMap * dmap = new SampledDensityMap(ps, resolution,
         apix,rad_key,weight_key,sig_cuttoff);
  return dmap;
}


IMPEM_END_NAMESPACE
