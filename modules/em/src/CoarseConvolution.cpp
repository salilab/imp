/**
 *  \file CoarseConvolution.cpp
 *  \brief Convolutes two grids
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/em/CoarseConvolution.h>
#include <IMP/em/internal/map_overlap.h>
#include <cmath>

IMPEM_BEGIN_NAMESPACE

float CoarseConvolution::convolution(const DensityMap *f, DensityMap *g,
                                     float voxel_data_threshold, bool) {
  const DensityHeader *f_header = f->get_header();
  const DensityHeader *g_header = g->get_header();
  const double *f_data = f->get_data();
  const double *g_data = g->get_data();

  // validity checks
  IMP_USAGE_CHECK(
      f->same_dimensions(g),
      "This function cannot handle density maps of different size. "
          << "First map dimensions : " << f_header->get_nx() << " x "
          << f_header->get_ny() << " x " << f_header->get_nz()
          << "; Second map dimensions: " << g_header->get_nx() << " x "
          << g_header->get_ny() << " x " << g_header->get_nz());
  IMP_USAGE_CHECK(
      f->same_voxel_size(g),
      "This function cannot handle density maps of different pixelsize. "
          << "First map pixelsize : " << f_header->get_spacing()
          << "; Second map pixelsize: " << g_header->get_spacing());
  bool same_origin = f->same_origin(g);
  int nvox = f_header->get_number_of_voxels();
  double conv = 0.0;

  if (same_origin) {  // Fastest version
    for (int i = 0; i < nvox; i++) {
      if ((g_data[i] > voxel_data_threshold) &&
          (f_data[i] > voxel_data_threshold)) {
        conv += f_data[i] * g_data[i];
      }
    }
  } else {  // Compute the CCC taking into account the different origins
    // Only consider voxels of g that lie within f
    internal::for_each_overlapping_voxel(
        f_header, g_header, [&](long ifh, long igh) {
          if (g_data[igh] > voxel_data_threshold &&
              f_data[ifh] > voxel_data_threshold) {
            conv += f_data[ifh] * g_data[igh];
          }
        });
  }
  return conv;
}

IMPEM_END_NAMESPACE
