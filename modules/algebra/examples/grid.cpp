/**
 *  \example grid.cpp
 *  \brief Show some of the basics of using a grid from C++.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/algebra/standard_grids.h>
#include <IMP/algebra/vector_generators.h>
#include <IMP/algebra/grid_utility.h>
#include <boost/foreach.hpp>
#include <algorithm>

int main(int, char * []) {
  IMP::algebra::BoundingBox3D bb(IMP::algebra::Vector3D(0, 0, 0),
                                 IMP::algebra::Vector3D(10, 10, 10));
  // declare a grid covering the space 0 to 100 with a cell size of 1 and
  // default value of 0
  // unfortunately, due to swig GridD takes lots of redundant template
  // parameters
  typedef IMP::algebra::GridD<3, IMP::algebra::DenseGridStorageD<3, double>,
                              double> Grid;
  Grid grid(1, bb, 0);
  // fill it with a gradient
  BOOST_FOREACH(Grid::Index i, grid.get_all_indexes()) {
    IMP::algebra::Vector3D c = grid.get_center(i);
    grid[i] = IMP::algebra::get_distance(c, IMP::algebra::Vector3D(10, 10, 10));
  }
  // we can get smooth values at off-grid points
  // it is boring below .5
  std::cout << "Smooth" << std::endl;
  for (double x = .5; x < 4; x += .1) {
    double vo = IMP::algebra::get_linearly_interpolated(
        grid, IMP::algebra::Vector3D(x, x, x));
    std::cout << vo << " ";
  }
  std::cout << std::endl;
  // we can get chunky values at off-grid points instead
  std::cout << "Chunky" << std::endl;
  for (double x = .5; x < 4; x += .1) {
    double vo = grid[IMP::algebra::Vector3D(x, x, x)];
    std::cout << vo << " ";
  }
  std::cout << std::endl;
  return EXIT_SUCCESS;
}
