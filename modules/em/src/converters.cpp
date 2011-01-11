/**
 *  \file converters.cpp
 *  \brief impl
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
*/

#include <IMP/em/converters.h>
#include <IMP/em/Voxel.h>
#include <IMP/em/CoarseCC.h>
IMPEM_BEGIN_NAMESPACE


std::vector<algebra::VectorD<3> > density2vectors(DensityMap *dmap,
                                                  Float threshold) {
  std::vector<algebra::VectorD<3> > vecs;
  const DensityHeader* header=dmap->get_header();
  long ind;
  for(int i=0;i<header->get_nx();i++){
    for(int j=0;j<header->get_ny();j++){
      for(int k=0;k<header->get_nz();k++){
        ind = dmap->xyz_ind2voxel(i,j,k);
        if(dmap->get_value(ind)>threshold) {
          vecs.push_back(algebra::Vector3D(
                            dmap->get_location_in_dim_by_voxel(ind,0),
                            dmap->get_location_in_dim_by_voxel(ind,1),
                            dmap->get_location_in_dim_by_voxel(ind,2)));
        }
      }
     }
  }//i
  return vecs;
}


Particles density2particles(DensityMap *dmap, Float threshold,
                            Model *m,int step) {
  Particles ps;
  Float x,y,z,val;
  Float voxel_size=dmap->get_header()->get_spacing();
  Float r=sqrt(3.*voxel_size*voxel_size);
  FloatKey dens_key = Voxel::get_density_key();
  long ind;
  const DensityHeader *dhed = dmap->get_header();
  for(int i=0;i<dhed->get_nx();i=i+1*step){
    for(int j=0;j<dhed->get_ny();j=j+1*step){
      for(int k=0;k<dhed->get_nz();k=k+1*step){
        ind = dmap->xyz_ind2voxel(i,j,k);
        val = dmap->get_value(ind);
        if (val > threshold) {
          IMP_NEW(Particle,p,(m));
          x = dmap->get_location_in_dim_by_voxel(ind,0);
          y = dmap->get_location_in_dim_by_voxel(ind,1);
          z = dmap->get_location_in_dim_by_voxel(ind,2);
          Voxel::setup_particle(p,IMP::algebra::VectorD<3>(x,y,z),r,val);
          ps.push_back(p);
        }
      }//k
    }//j
  }//i
  return ps;
}


SampledDensityMap * particles2density(
   const Particles &ps,
   Float resolution, Float apix,
   int sig_cuttoff,
   const FloatKey &weight_key)
 {
   IMP::Pointer<SampledDensityMap> dmap(new SampledDensityMap(
                                     ps, resolution,
                                     apix,weight_key,sig_cuttoff));
   return dmap.release();
}

SurfaceShellDensityMap * particles2surface(
   const Particles &ps,Float apix,
   const FloatKey &weight_key){
  IMP::Pointer<SurfaceShellDensityMap> dmap(new SurfaceShellDensityMap(
                                     ps,
                                     apix,weight_key));
   return dmap.release();
}
//TODO - more this function to another file

Float calculate_intersection_score(const SurfaceShellDensityMap *d1,
                                   const SurfaceShellDensityMap *d2) {
  //calculate the correlation between the maps
  //voxels with value 1 are surface and higher values are internal voxels
  //first check that the bounding boxes of the maps are intersecting
  return CoarseCC::cross_correlation_coefficient(d1,d2,1.+EPS,true);
}

IMPEM_END_NAMESPACE
