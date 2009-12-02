/**
 *  \file converters.cpp
 *  \brief impl
 *  Copyright 2007-9 Sali Lab. All rights reserved.
*/

#include <IMP/em/converters.h>
#include <IMP/em/Voxel.h>

IMPEM_BEGIN_NAMESPACE


void density2particles(DensityMap &dmap, Float threshold,
                              Particles &ps, Model *m) {
  Float x,y,z,val;
  Float voxel_size=dmap.get_header()->Objectpixelsize;
  Float r=sqrt(3.*voxel_size*voxel_size);
  FloatKey dens_key = Voxel::get_density_key();
  for (long i=0;i<dmap.get_number_of_voxels();i++) {
    val = dmap.get_value(i);
    if (val > threshold) {
      Particle * p = new Particle(m);
      x = dmap.voxel2loc(i,0);
      y = dmap.voxel2loc(i,1);
      z = dmap.voxel2loc(i,2);
      Voxel::setup_particle(p,IMP::algebra::Vector3D(x,y,z),r,val);
      ps.push_back(p);
    }
  }
}

IMPEM_END_NAMESPACE
