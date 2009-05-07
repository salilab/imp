/**
 * Copyright 2007-9 Sali Lab. All rights reserved.
 */
#include <IMP.h>
#include <IMP/core.h>
#include <IMP/algebra.h>
#include <IMP/atom.h>
#include <boost/timer.hpp>

using namespace IMP;
using namespace IMP::core;
using namespace IMP::algebra;
using namespace IMP::atom;


int main(int argc, char **argv) {

  std::vector<Vector3D> vs(10000);
  for (unsigned int i=0; i< vs.size(); ++i) {
    vs[i]= random_vector_in_unit_sphere<3>();
  }
  {
    // TEST 1
    double runtime;
    // measure time
    Rotation3D r= random_rotation();
    IMP_TIME(
             {
               for (unsigned int i=0; i< vs.size(); ++i) {
                 vs[i]= r.rotate(vs[i]);
               }
             }, runtime);
    std::cout << "TEST1 (cache)  took " << runtime
              << std::endl;
  }
  Vector3D sum(0,0,0);
  for (unsigned int i=0; i< vs.size(); ++i) {
    sum+= vs[i];
  }
{
    // TEST 2
    double runtime;
    // measure time
    Rotation3D r= random_rotation();
    IMP_TIME(
             {
               for (unsigned int i=0; i< vs.size(); ++i) {
                 vs[i]= r.rotate_no_cache(vs[i]);
               }
             }, runtime);
    std::cout << "TEST1 (no cache)  took " << runtime
              << std::endl;
  }
  for (unsigned int i=0; i< vs.size(); ++i) {
    sum+= vs[i];
  }
  std::cout << "Sum is " << sum << std::endl;
  return 0;
}
