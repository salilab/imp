/**
 *  \file CoarseConvolution.cpp
 *  \brief Convolutes two grids
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/em/CoarseConvolution.h>
#include <math.h>

IMPEM_BEGIN_NAMESPACE

float CoarseConvolution::convolution(const DensityMap &f, DensityMap &g,
                                     float voxel_data_threshold,
                                     bool recalc_ccnormfac)
{
  const DensityHeader *f_header = f.get_header();
  const DensityHeader *g_header = g.get_header();
  const emreal *f_data = f.get_data();
  const emreal *g_data = g.get_data();

  //validity checks
  IMP_check(f.same_dimensions(g),
            "This function cannot handle density maps of different size. "
            << "First map dimensions : " << f_header->nx << " x "
            << f_header->ny << " x " << f_header->nz
            << "; Second map dimensions: " << g_header->nx << " x "
            << g_header->ny << " x " << g_header->nz,
            InvalidStateException);
  IMP_check(f.same_voxel_size(g),
            "This function cannot handle density maps of different pixelsize. "
            << "First map pixelsize : " << f_header->Objectpixelsize
            << "; Second map pixelsize: " << g_header->Objectpixelsize,
            InvalidStateException);
  bool same_origin = f.same_origin(g);
  int  nvox = f_header->nx*f_header->ny*f_header->nz;
  emreal conv = 0.0;

  if(same_origin){ // Fastest version
    for (int i=0;i<nvox;i++) {
      if ((g_data[i] > voxel_data_threshold)
              && (f_data[i] > voxel_data_threshold)) {
        conv += f_data[i]*g_data[i];
      }
    }
  }
  else  { // Compute the CCC taking into account the different origins
    // Given the same size of the maps and the dimension order, the difference
    // between two positions in voxels is always the same

    // calculate the difference in voxels between the origin of the  model map
    // and the origin of the em map.
    float voxel_size = f_header->Objectpixelsize;
    int ivoxx_shift = (int)floor((g_header->get_xorigin()
                                  - f_header->get_xorigin())
                                 / voxel_size);
    int ivoxy_shift = (int)floor((g_header->get_yorigin()
                                  - f_header->get_yorigin())
                                 / voxel_size);
    int ivoxz_shift = (int)floor((g_header->get_zorigin()
                                  - f_header->get_zorigin())
                                 / voxel_size);


    int j; // Index for em data
    int i; // Index for model data
    // calculate the shift in index of the origin of model_map in em_map
    // ( j can be negative)
    j = ivoxz_shift * f_header->nx * f_header->ny + ivoxy_shift
        * f_header->nx + ivoxx_shift;
    for (i=0;i<nvox;i++) {
      // if the voxel of the model is above the threshold
      if (g_data[i] > voxel_data_threshold) {
        if (j + i >= 0 && j + i < nvox)  {
          if (f_data[j+i] > voxel_data_threshold){
            conv += f_data[j+i] * g_data[i];
          }
        }
      }
    }
  }
  return conv;
}

IMPEM_END_NAMESPACE
