/**
 *  \file test_get_grid.cpp
 *  \brief Test conversion of a DensityMap to a grid.
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/em/DensityMap.h>
#include <IMP/flags.h>
#include <IMP/exception.h>
#include <cmath>

int main(int argc, char *argv[]) {
  IMP::setup_from_argv(argc, argv, "Test get_grid.");

  // Use a map with nontrivial origin and spacing
  IMP::Pointer<IMP::em::DensityMap> m =
      IMP::em::create_density_map(4, 5, 6, 2.0);
  m->set_origin(IMP::algebra::Vector3D(10., 20., 30.));
  for (long i = 0; i < m->get_number_of_voxels(); ++i) {
    m->set_value(i, static_cast<double>(i));
  }

  auto g = IMP::em::get_grid(m);
  for (int x = 0; x < 4; ++x) {
    for (int y = 0; y < 5; ++y) {
      for (int z = 0; z < 6; ++z) {
        IMP::algebra::ExtendedGridIndexD<3> ei(x, y, z);
        double grid_value = g[g.get_index(ei)];
        double map_value = m->get_value(m->xyz_ind2voxel(x, y, z));
        IMP_ALWAYS_CHECK(std::abs(grid_value - map_value) < 1e-6,
                         "Grid value " << grid_value << " at " << x << ","
                         << y << "," << z << " does not match map value "
                         << map_value, IMP::ValueException);
      }
    }
  }
  return EXIT_SUCCESS;
}
