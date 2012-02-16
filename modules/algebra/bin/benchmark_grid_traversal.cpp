/**
 * Copyright 2007-2012 IMP Inventors. All rights reserved.
 */
#include <IMP/benchmark/benchmark_config.h>
#include <IMP/benchmark/benchmark_macros.h>
#include <IMP/benchmark/utility.h>
#include <IMP/algebra/GridD.h>
#include <IMP/algebra/grid_utility.h>
#include <IMP/algebra/standard_grids.h>

struct Sum {
  double sum;
  Sum(): sum(0){}
  template <class G, class I, class V>
  void operator()(const G &g, const I &i,
                  const V &) {
    sum+=g[i];
  }
};

template <class Grid>
double do_foreach(const Grid &g) {
  double dist=0;
  IMP_GRID3D_FOREACH_VOXEL(g, {
      dist+=g[voxel_index];
    });
  return dist;
}

template <class Grid>
void benchmark(const Grid &g,
               std::string name) {
  {
    double runtime, dist=0;
    IMP_TIME(
             {
               dist+=g.apply(Sum()).sum;
             }, runtime);
    IMP::benchmark::report(std::string("grid apply ")+name,
                           runtime, dist);
  }
  {
    double runtime, dist=0;
    IMP_TIME(
             dist+=do_foreach(g), runtime);
    IMP::benchmark::report(std::string("grid foreach ")+name,
                           runtime, dist);
  }
  {
   double runtime, dist=0;
    IMP_TIME(
             for (typename Grid::AllIndexIterator
                    it = g.all_indexes_begin(); it != g.all_indexes_end();
                  ++it) {
               dist +=g[*it];
             }, runtime);
    IMP::benchmark::report(std::string("grid iterator ")+name,
                           runtime, dist);
  }
}

int main(int, char **) {
  using namespace IMP::algebra;
  BoundingBox3D bb(Vector3D(0,0,0),
                   Vector3D(100, 100, 100));
  {
    GridD<3, DenseGridStorageD<3, double>, double, DefaultEmbeddingD<3> >
      grid(1, bb, 0);
    grid[Vector3D(50,25,30)]=1;
    benchmark(grid, "dense 100");
  }
  return 0;
}
