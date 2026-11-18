/**
 *  \file IMP/em/internal/map_overlap.h
 *  \brief Iterate over the voxels common to two density maps.
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM_INTERNAL_MAP_OVERLAP_H
#define IMPEM_INTERNAL_MAP_OVERLAP_H

#include <IMP/em/em_config.h>
#include <IMP/em/DensityHeader.h>
#include <algorithm>
#include <cmath>

IMPEM_BEGIN_INTERNAL_NAMESPACE

//! Call f(i1, i2) for each pair of voxels at the same position in two maps
/** i1 is the index of the voxel in the first map, and i2 the index of the
    corresponding voxel in the second map. Both maps are assumed to have
    the same spacing; the offset between the two origins is rounded to the
    nearest whole number of voxels. The maps may have different dimensions.
    Voxels of the second map that lie outside of the first map are skipped
    (each axis is handled separately, so voxels near a face of one map are
    never paired with voxels on the opposite face of the other).
    Pairs are visited in order of increasing i2.
 */
template <class F>
inline void for_each_overlapping_voxel(const DensityHeader *h1,
                                       const DensityHeader *h2, F f) {
  double spacing = h1->get_spacing();
  int shift[3] = {
      static_cast<int>(
          std::lround((h2->get_xorigin() - h1->get_xorigin()) / spacing)),
      static_cast<int>(
          std::lround((h2->get_yorigin() - h1->get_yorigin()) / spacing)),
      static_cast<int>(
          std::lround((h2->get_zorigin() - h1->get_zorigin()) / spacing))};
  int n1x = h1->get_nx(), n1y = h1->get_ny(), n1z = h1->get_nz();
  int n2x = h2->get_nx(), n2y = h2->get_ny(), n2z = h2->get_nz();
  // Range of x indices in the second map that lie within the first
  int xlo = std::max(0, -shift[0]);
  int xhi = std::min(n2x, n1x - shift[0]);
  if (xlo >= xhi) return;
  for (int z2 = 0; z2 < n2z; ++z2) {
    int z1 = z2 + shift[2];
    if (z1 < 0 || z1 >= n1z) continue;
    for (int y2 = 0; y2 < n2y; ++y2) {
      int y1 = y2 + shift[1];
      if (y1 < 0 || y1 >= n1y) continue;
      long row2 = (static_cast<long>(z2) * n2y + y2) * n2x;
      long row1 = (static_cast<long>(z1) * n1y + y1) * n1x + shift[0];
      for (int x2 = xlo; x2 < xhi; ++x2) {
        f(row1 + x2, row2 + x2);
      }
    }
  }
}

IMPEM_END_INTERNAL_NAMESPACE

#endif /* IMPEM_INTERNAL_MAP_OVERLAP_H */
