/**
 * Copyright 2007-2010 Sali Lab. All rights reserved.
 */
#include <IMP.h>
#include <IMP/core.h>
#include <IMP/algebra.h>
#include <IMP/atom.h>
#include <boost/timer.hpp>
#include <IMP/benchmark/utility.h>
#include <IMP/benchmark/macros.h>

using namespace IMP;
using namespace IMP::core;
using namespace IMP::algebra;
using namespace IMP::atom;


int main(int argc, char **argv) {
  set_log_level(SILENT);
  set_check_level(IMP::NONE);
  std::vector<VectorD<3> > vs(10000);
  for (unsigned int i=0; i< vs.size(); ++i) {
    vs[i]= get_random_vector_in(get_unit_sphere_d<3>());
  }
  {
    // TEST 1
    double runtime;
    // measure time
    Rotation3D r= get_random_rotation_3d();
    double sum=0;
    IMP_TIME(
             {
               for (unsigned int i=0; i< vs.size(); ++i) {
                 vs[i]= r.get_rotated(vs[i]);
                 sum+= vs[i][0]+vs[i][1]+vs[i][2];
               }
             }, runtime);
    IMP::benchmark::report("rotation (cache)", runtime, sum);
  }
  VectorD<3> sum(0,0,0);
  for (unsigned int i=0; i< vs.size(); ++i) {
    sum+= vs[i];
  }
{
    // TEST 2
    double runtime;
    // measure time
    Rotation3D r= get_random_rotation_3d();
    double sum=0;
    IMP_TIME(
             {
               for (unsigned int i=0; i< vs.size(); ++i) {
                 vs[i]= r.get_rotated_no_cache(vs[i]);
                 sum+= vs[i][0]+vs[i][1]+vs[i][2];
               }
             }, runtime);
    IMP::benchmark::report("rotation (nocache)", runtime,
                           sum);
  }
  //std::cout << "Sum is " << sum << std::endl;
  return 0;
}
