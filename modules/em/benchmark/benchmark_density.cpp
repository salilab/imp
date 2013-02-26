/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */
#include <IMP.h>
#include <IMP/benchmark.h>
#include <IMP/em.h>
#include <IMP/algebra.h>
#include <IMP/base/flags.h>
using namespace IMP;
using namespace IMP::em;
using namespace IMP::benchmark;
using namespace IMP::algebra;


namespace {
void do_benchmark() {
  IMP::Pointer<DensityMap> m[3];
  for (unsigned int i=0; i< 3; ++i) {
    m[i]= new DensityMap();
    unsigned int n[3];
    double spacing=(i+3.0)/100.0;
    algebra::Vector3D wid(i+3,i+3,i+3);
    for (unsigned int j=0; j< 3; ++j) {
      n[j]= static_cast<int>(std::ceil(wid[j]/spacing));
    }
    m[i]->set_void_map(n[0], n[1], n[2]);
    m[i]->set_origin(algebra::Vector3D(0,0,0));
    m[i]->update_voxel_size(spacing);
  }
  unsigned int N=30;
  {
    double runtime, dist=0;
    // measure time
    IMP_TIME_N(
               {for (unsigned int i=0; i< 3; ++i ) {
                   double spacing=(i+3.0)/100.0;
                   for ( int j=0;
                        j< m[i]->get_header()->get_nx(); ++j) {
                     double x= m[i]->get_origin()[0]+j*spacing;
                     for ( int k=0;
                          k< m[i]->get_header()->get_ny(); ++k) {
                       double y= m[i]->get_origin()[1]+k*spacing;
                       for ( int l=0;
                            l< m[i]->get_header()->get_nz(); ++l) {
                         double z= m[i]->get_origin()[2]+l*spacing;
                         dist+= x+y+z;
                       }
                     }
                   }
                 }
               }, runtime, N);
    IMP::benchmark::report("density loop", "uncached", runtime, dist);
  }
  {
    double runtime, dist=0;
    // measure time
    IMP_TIME_N(
               {for (unsigned int i=0; i< 3; ++i ) {
                   for (int j=0;
                        j< m[i]->get_number_of_voxels(); ++j) {
                     dist+= m[i]->get_location_in_dim_by_voxel(j,0)
                       + m[i]->get_location_in_dim_by_voxel(j,1)
                       + m[i]->get_location_in_dim_by_voxel(j,2);
                   }
                 }
               }, runtime, N);
    IMP::benchmark::report("density loop", "cached", runtime, dist);
  }
}
}




int main(int argc, char **argv) {
  IMP::base::setup_from_argv(argc, argv, "Benchmark scanning density maps");
  do_benchmark();
  return IMP::benchmark::get_return_value();
}
