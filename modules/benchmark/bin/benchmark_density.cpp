/**
 * Copyright 2007-2010 IMP Inventors. All rights reserved.
 */
#include <IMP.h>
#include <IMP/em.h>
#include <IMP/benchmark.h>
#include <IMP/algebra.h>
using namespace IMP;
using namespace IMP::em;
using namespace IMP::benchmark;
using namespace IMP::algebra;


void do_benchmark(double *targets) {
  IMP::Pointer<DensityMap> m[3];
  for (unsigned int i=0; i< 3; ++i) {
    m[i]= new DensityMap();
    unsigned int n[3];
    double spacing=(i+3.0)/100.0;
    algebra::VectorD<3> wid(i+3,i+3,i+3);
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
    IMP::benchmark::report("uncached density loop", runtime, targets[0], dist);
  }
  {
    double runtime, dist=0;
    // measure time
    IMP_TIME_N(
               {for (unsigned int i=0; i< 3; ++i ) {
                   for (unsigned int j=0;
                        j< m[i]->get_number_of_voxels(); ++j) {
                     dist+= m[i]->get_location_in_dim_by_voxel(j,0)
                       + m[i]->get_location_in_dim_by_voxel(j,1)
                       + m[i]->get_location_in_dim_by_voxel(j,2);
                   }
                 }
               }, runtime, N);
    IMP::benchmark::report("cached density loop", runtime, targets[1], dist);
  }
}





int main(int , char **) {
  double targets[]={1.393333, 4.180000};
  do_benchmark(targets);
  return IMP::benchmark::get_return_value();
}
