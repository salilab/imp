/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/algebra/connolly_surface.h>
#include <IMP/algebra/vector_generators.h>
#include <IMP/algebra/Sphere3D.h>
#include <IMP/algebra/io.h>
#include <IMP/benchmark/utility.h>
#include <IMP/benchmark/benchmark_macros.h>
#include <IMP/base/flags.h>


int main(int argc, char **argv) {
  IMP::base::setup_from_argv(argc, argv, "Benchmark algorithms for surfaces");
  IMP::algebra::Sphere3Ds sps = IMP::algebra::read_spheres(
      IMP::algebra::get_data_path("benchmark_surface.spheres"));
  if (IMP::base::run_quick_test) {
    while (sps.size() > 100) sps.pop_back();
  }
#ifdef IMP_ALGEBRA_USE_IMP_CGAL
  {
     double result = 0;
     double runtime;
     typedef std::pair<double, double> DP;
     IMP_TIME({
                DP sav = IMP::algebra::get_surface_area_and_volume(sps);
                result += sav.first;
              },
              runtime);
     IMP::benchmark::report("surface", "cgal", runtime, result);
  }
#endif
 {
     double result = 0;
     double runtime;
     IMP_TIME({
                IMP::algebra::Vector3Ds vps =
                    IMP::algebra::get_uniform_surface_cover(sps, 1);
                result += vps.size();
              },
              runtime);
     IMP::benchmark::report("surface", "uniform cover", runtime, result);
  }
 {
     double result = 0;
     double runtime;
     IMP_TIME({
                IMP::algebra::ConnollySurfacePoints vps =
                    IMP::algebra::get_connolly_surface(sps, 1, 1);
                result += vps.size();
              },
              runtime);
     IMP::benchmark::report("surface", "connolly", runtime, result);
  }
  return IMP::benchmark::get_return_value();
}
