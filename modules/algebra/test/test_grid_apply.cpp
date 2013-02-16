/**
 *  \file test_grid.cpp   \brief A class to represent a voxel grid.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/algebra/standard_grids.h>
#include <IMP/algebra/vector_generators.h>
#include <IMP/base/set.h>
#include <IMP/test/test_macros.h>
#include <algorithm>

using namespace IMP::algebra;
typedef DenseGrid3D<int> Grid;

struct Accum {
  int sum_;
  Accum():sum_(0){}
  template <class G>
  void operator()(const G &g,
                  const typename G::Index& index,
                  const typename G::Vector& v) {
    sum_+=g[index];
    typename G::Vector vo=g.get_center(index);
    IMP_TEST_LESS_THAN((vo-v).get_magnitude(), .1);
  }
};

struct Count {
  IMP::base::set<Grid::Index> seen_;
  template <class G>
  void operator()(const G &,
                  const typename G::Index& index,
                  const typename G::Vector&) {
    seen_.insert(index);
  }
  int get_count() const {return seen_.size();}
};

int main(int, char *[]) {
  {
    BoundingBox3D bb(Vector3D(0,0,0), Vector3D(100,100,100));
    BoundingBox3D gbb(get_random_vector_in(bb));
    gbb+= get_random_vector_in(bb);
    Grid g(1, gbb, 0);
    unsigned int count= g.apply(Count()).get_count();
    IMP_TEST_EQUAL(count, g.get_number_of_voxels(0)*g.get_number_of_voxels(1)
           * g.get_number_of_voxels(2));
  }
  {
    BoundingBox3D bb(Vector3D(0,0,0), Vector3D(100,100,100));
    BoundingBox3D gbb(get_random_vector_in(bb));
    gbb+= get_random_vector_in(bb);
    Grid g(1, gbb, 0);
    for (unsigned int i=0; i< 5000; ++i) {
      Vector3D cur(get_random_vector_in(gbb));
      ++g[cur];
    }
    Accum out=g.apply(Accum());
    IMP_TEST_EQUAL(out.sum_,5000);
  }
  return EXIT_SUCCESS;
}
