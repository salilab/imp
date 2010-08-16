/**
 *  \file converters.cpp
 *  \brief impl
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#include <IMP/em/converters.h>
#include <IMP/em/Voxel.h>

IMPEM_BEGIN_NAMESPACE


std::vector<algebra::VectorD<3> > density2vectors(DensityMap &dmap,
                                                  Float threshold) {
  std::vector<algebra::VectorD<3> > vecs;
  const DensityHeader* header=dmap.get_header();
  long ind;
  for(int i=0;i<header->get_nx();i++){
    for(int j=0;j<header->get_ny();j++){
      for(int k=0;k<header->get_nz();k++){
        ind = dmap.xyz_ind2voxel(i,j,k);
        if(dmap.get_value(ind)>threshold) {
          vecs.push_back(algebra::Vector3D(
                            dmap.voxel2loc(ind,0),
                            dmap.voxel2loc(ind,1),
                            dmap.voxel2loc(ind,2)));
        }
      }
     }
  }//i
  return vecs;
}


Particles density2particles(DensityMap &dmap, Float threshold,
                            Model *m,int step) {
  Particles ps;
  Float x,y,z,val;
  Float voxel_size=dmap.get_header()->get_spacing();
  Float r=sqrt(3.*voxel_size*voxel_size);
  FloatKey dens_key = Voxel::get_density_key();
  long ind;
  const DensityHeader *dhed = dmap.get_header();
  for(int i=0;i<dhed->get_nx();i=i+1*step){
    for(int j=0;j<dhed->get_ny();j=j+1*step){
      for(int k=0;k<dhed->get_nz();k=k+1*step){
        ind = dmap.xyz_ind2voxel(i,j,k);
        val = dmap.get_value(ind);
        if (val > threshold) {
          Particle * p = new Particle(m);
          x = dmap.voxel2loc(ind,0);
          y = dmap.voxel2loc(ind,1);
          z = dmap.voxel2loc(ind,2);
          Voxel::setup_particle(p,IMP::algebra::VectorD<3>(x,y,z),r,val);
          ps.push_back(p);
        }
      }//k
    }//j
  }//i
  return ps;
}


IMPEMEXPORT SampledDensityMap * particles2density(
   const Particles &ps,
   Float resolution, Float apix,
   int sig_cuttoff,
   const FloatKey &rad_key,
   const FloatKey &weight_key)
 {
   IMP::Pointer<SampledDensityMap> dmap(new SampledDensityMap(
                                     ps, resolution,
                                     apix,rad_key,weight_key,sig_cuttoff));
   return dmap.release();
}


IMPEM_END_NAMESPACE
