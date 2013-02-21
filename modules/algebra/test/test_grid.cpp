/**
 *  \file test_grid.cpp   \brief A class to represent a voxel grid.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/algebra/standard_grids.h>
#include <IMP/algebra/vector_generators.h>
#include <algorithm>

using namespace IMP::algebra;
typedef DenseGrid3D<int> Grid;

namespace {
bool intersects(BoundingBox3D a, BoundingBox3D b) {
  Vector3D mn(std::max(a.get_corner(0)[0], b.get_corner(0)[0]),
              std::max(a.get_corner(0)[1], b.get_corner(0)[1]),
              std::max(a.get_corner(0)[2], b.get_corner(0)[2]));
  Vector3D mx(std::min(a.get_corner(1)[0], b.get_corner(1)[0]),
              std::min(a.get_corner(1)[1], b.get_corner(1)[1]),
              std::min(a.get_corner(1)[2], b.get_corner(1)[2]));
  return (mn[0] <= mx[0] && mn[1] <= mx[1] && mn[2] <= mx[2]);
}

bool test_marked(const Grid &g, BoundingBox3D bb) {
  for (Grid::IndexIterator it= g.all_indexes_begin();
       it != g.all_indexes_end(); ++it) {
    BoundingBox3D cbb= g.get_bounding_box(*it);
    if (intersects(cbb, bb)) {
      if (g[*it] != 1) {
        std::cout << "Cell " << *it << " was not marked as being part of "
                  << bb << ". It has a bounding box of " << cbb
                  << " and the grid has one of " << g.get_bounding_box()
                  << std::endl;
        return false;
      }
    }
  }
  return true;
}

}

int main(int, char *[]) {
  BoundingBox3D bb(Vector3D(0,0,0), Vector3D(100,100,100));
  BoundingBox3D gbb(get_random_vector_in(bb));
  gbb+= get_random_vector_in(bb);
  Grid g(1, gbb, 0);
  for (unsigned int i=0; i< 5; ++i) {
    std::cout << i << std::endl;
    BoundingBox3D qbb(get_random_vector_in(gbb));
    qbb+= get_random_vector_in(gbb);
    for (Grid::VoxelIterator it= g.voxels_begin(qbb);
         it != g.voxels_end(qbb); ++it) {
      *it=1;
    }
    if (!test_marked(g, qbb)) return EXIT_FAILURE;
    std::fill(g.all_voxels_begin(), g.all_voxels_end(), 0);
  }
  return EXIT_SUCCESS;
}
