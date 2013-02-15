/**
 *  \file envelope_penetration.cpp
 *  \brief functions for calculation envelope penetration
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/em/envelope_penetration.h>
#include <IMP/core/XYZ.h>
IMPEM_BEGIN_NAMESPACE

long get_number_of_particles_outside_of_the_density(DensityMap *dmap,
     const Particles &ps,const IMP::algebra::Transformation3D &t,float thr) {
  IMP_LOG_VERBOSE("start calculating the how many particles out of "
         <<ps.size()<<" in density"<< std::endl);
  long out_of_dens=0;
  core::XYZs xyz = core::XYZs(ps);
  for (unsigned int i = 0; i < ps.size(); ++i) {
    IMP::algebra::Vector3D vec = t.get_transformed(xyz[i].get_coordinates());
    if (!dmap->is_part_of_volume(vec)) {
      IMP_LOG_VERBOSE("position: "<<vec<<" is out of density"<<std::endl);
      out_of_dens++;
    }
    else {
      if (dmap->get_value(vec)<thr) {
        IMP_LOG_VERBOSE("position: "<<vec<<" has density value "<<
                dmap->get_value(vec)<<
                " with is lower than required"<<std::endl);
        out_of_dens++;
      }
    }
  }
  IMP_LOG_VERBOSE("the number of particles outside of the density is:"
          << out_of_dens << std::endl);
  std::cout<<"the number of particles outside of the density is:"
          << out_of_dens << std::endl;
  return out_of_dens;
}

Ints get_numbers_of_particles_outside_of_the_density(
        DensityMap *dmap,const Particles &ps,
        const IMP::algebra::Transformation3Ds &transforms,float thr) {
  IMP_LOG_VERBOSE("start calculating the how many particles out of "
         <<ps.size()<<" in density"<< std::endl);
  Ints results;
  for (algebra::Transformation3Ds::const_iterator it =
         transforms.begin(); it != transforms.end();it++) {
    results.push_back(
      get_number_of_particles_outside_of_the_density(dmap,ps,*it,thr));
  }
  return results;
}

double get_percentage_of_voxels_covered_by_particles(
     DensityMap *dmap,const Particles &ps,
     float smoothing_radius,
     const IMP::algebra::Transformation3D &t,float thr) {
  IMP_NEW(DensityMap,dmap_ind,(*(dmap->get_header())));
  dmap_ind->reset_data();
  emreal* data = dmap->get_data();
  emreal* new_data = dmap_ind->get_data();
  IMP_LOG_VERBOSE("START get_percentage_of_voxels_covered_by_particles"
         << std::endl);
  int iminx, iminy, iminz, imaxx, imaxy, imaxz,ivoxz,ivoxy,ivoxx,znxny,ivox;
  int nxny=dmap->get_header()->get_nx()*dmap->get_header()->get_ny();
  int nx=dmap->get_header()->get_nx();
  core::XYZs xyz = core::XYZs(ps);
  for (unsigned int i = 0; i < ps.size(); ++i) {
    IMP::algebra::Vector3D vec = t.get_transformed(xyz[i].get_coordinates());
     calc_local_bounding_box(
       dmap,
       vec[0],vec[1],vec[2],
       smoothing_radius*2,
       iminx, iminy, iminz, imaxx, imaxy, imaxz);
     for (ivoxz=iminz;ivoxz<=imaxz;ivoxz++) {
       znxny=ivoxz * nxny;
       for (ivoxy=iminy;ivoxy<=imaxy;ivoxy++)  {
         // we increment ivox this way to avoid unneceessary multiplication
         // operations.
         ivox = znxny + ivoxy * nx + iminx;
         for (ivoxx=iminx;ivoxx<=imaxx;ivoxx++) {
           if (data[ivox]<thr) continue;
           new_data[ivox]=1;
           ivox++;
         }
       }
     }
  }
  //get number of particles above a threshold:
  long voxs_t=0;
  for (long ii=0;ii<dmap->get_number_of_voxels();ii++) {
    if (data[ii]>=thr) ++voxs_t;
  }
  return get_sum(dmap_ind)/voxs_t;
}
IMPEM_END_NAMESPACE
