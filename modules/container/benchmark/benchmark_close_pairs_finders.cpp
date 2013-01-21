/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP.h>
#include <IMP/core.h>
#include <IMP/algebra.h>
#include <IMP/benchmark/utility.h>
#include <IMP/benchmark/benchmark_macros.h>
#include <IMP/base/flags.h>
#include <IMP/container.h>

using namespace IMP;
using namespace IMP::core;
using namespace IMP::algebra;
using namespace IMP::benchmark;
using namespace IMP::container;

namespace {
void test_one(std::string name,
              ClosePairsFinder *cpf, unsigned int n,
              float rmin, float rmax,
              bool nobi=false) {
  set_log_level(SILENT);
  set_check_level(IMP::NONE);
  {
    Vector3D minc(0,0,0), maxc(10,10,10);
    IMP_NEW(Model, m, ());
    ParticlesTemp ps = create_xyzr_particles(m, n, rmin);
    ParticleIndexes psi= IMP::internal::get_index(ps);
    ::boost::uniform_real<> rand(rmin, rmax);
    for (unsigned int i=0; i< ps.size(); ++i) {
      XYZ(ps[i])
        .set_coordinates(get_random_vector_in(BoundingBox3D(minc, maxc)));
      XYZR(ps[i]).set_radius(rand(random_number_generator));
    }
    cpf->set_distance(0);
    double result=0;
    double runtime;
    IMP_TIME({
        result+=cpf->get_close_pairs(m, psi).size();
      }, runtime);
    std::ostringstream oss;
    oss << "cpf index " << name << " " << n << " " << rmax;
    report(oss.str(), runtime, result);
  }
{
    Vector3D minc(0,0,0), maxc(10,10,10);
    IMP_NEW(Model, m, ());
    ParticlesTemp ps = create_xyzr_particles(m, n, rmin);
    ::boost::uniform_real<> rand(rmin, rmax);
    for (unsigned int i=0; i< ps.size(); ++i) {
      XYZ(ps[i])
        .set_coordinates(get_random_vector_in(BoundingBox3D(minc, maxc)));
      XYZR(ps[i]).set_radius(rand(random_number_generator));
    }
    cpf->set_distance(0);
    double result=0;
    double runtime;
    IMP_TIME({
        result+=cpf->get_close_pairs(ps).size();
      }, runtime);
    std::ostringstream oss;
    oss << "cpf " << name << " " << n << " " << rmax;
    report(oss.str(), runtime, result);
  }
  if (!nobi) {
    Vector3D minc(0,0,0), maxc(10,10,10);
    IMP_NEW(Model, m, ());
    ParticlesTemp ps0 = create_xyzr_particles(m, n, rmin);
    ParticlesTemp ps1 = create_xyzr_particles(m, n, rmin);
    ParticleIndexes ps0i= IMP::internal::get_index(ps0);
    ParticleIndexes ps1i= IMP::internal::get_index(ps1);
    ::boost::uniform_real<> rand(rmin, rmax);
    for (unsigned int i=0; i< ps0.size(); ++i) {
      XYZ(ps0[i])
        .set_coordinates(get_random_vector_in(BoundingBox3D(minc, maxc)));
      XYZR(ps0[i]).set_radius(rand(random_number_generator));
    }
    for (unsigned int i=0; i< ps1.size(); ++i) {
      XYZ(ps1[i])
        .set_coordinates(get_random_vector_in(BoundingBox3D(minc, maxc)));
      XYZR(ps1[i]).set_radius(rand(random_number_generator));
    }
    cpf->set_distance(0);
    double result=0;
    double runtime;
    IMP_TIME({
        result+=cpf->get_close_pairs(m, ps0i, ps1i).size();
      }, runtime);
    std::ostringstream oss;
    oss << "bcpf " << name << " " << n << " " << rmax;
    report(oss.str(), runtime, result);
  }
}
}

int main(int argc, char **argv) {
  IMP::base::setup_from_argv(argc, argv, "Benchmark finding close pairs");
  {
    IMP_NEW(GridClosePairsFinder, cpf, ());
    std::string name="grid";
    test_one(name, cpf, 10, 0, .1, true);
    test_one(name, cpf, 100, 0, .1, true);
    test_one(name, cpf, 1000, 0, .1, true);
    test_one(name, cpf, 1000, 0, .5, true);
    test_one(name, cpf, 1000, 0, 5, true);
    test_one(name, cpf, 10000, 0, .1, true);
    test_one(name, cpf, 10000, 0, .5, true);
    test_one(name, cpf, 10000, 0, 5, true);
    //test_one(name, cpf, 100000, 0, .01, 18.648000, true);
    //test_one(name, cpf, 100000, 0, .1, 23.217500, true);
    //test_one(name, cpf, 100000, 0, .3, 51.800000, true);
  }
  {
    IMP_NEW(NearestNeighborsClosePairsFinder, cpf, ());
    std::string name="nn";
    test_one(name, cpf, 1000, 0, .1);
    test_one(name, cpf, 1000, 0, .5);
    test_one(name, cpf, 1000, 0, 5);
    test_one(name, cpf, 10000, 0, .1);
    test_one(name, cpf, 10000, 0, .5);
    //test_one(name, cpf, 10000, 0, 5);
    //test_one(name, cpf, 100000, 0, .01, 42.624000);
    //test_one(name, cpf, 100000, 0, .1, 85.593333);
    //test_one(name, cpf, 100000, 0, .3, 329.300000);
  }
#ifdef IMP_BENCHMARK_USE_IMP_CGAL
  {
    IMP_NEW(BoxSweepClosePairsFinder, cpf, ());
    std::string name="box";
    // bi takes twice as long as non-bi
    test_one(name, cpf, 1000, 0, .1);
    test_one(name, cpf, 1000, 0, .5);
    test_one(name, cpf, 1000, 0, 5);
    test_one(name, cpf, 10000, 0, .1);
    test_one(name, cpf, 10000, 0, .5);
    test_one(name, cpf, 10000, 0, 5);
    //test_one(name, cpf, 100000, 0, .01, 44.696000);
    //test_one(name, cpf, 100000, 0, .1, 95.830000);
    //test_one(name, cpf, 100000, 0, .3, 198.320000);
  }
#endif
  {
    IMP_NEW(QuadraticClosePairsFinder, cpf, ());
    //std::cout << "Quadratic:" << std::endl;
    // bi also twice as as slow
    test_one("quadratic", cpf, 10, 0, .1);
    test_one("quadratic", cpf, 100, 0, .1);
    test_one("quadratic", cpf, 1000, 0, .1);
    test_one("quadratic", cpf, 1000, 0, .5);
    test_one("quadratic", cpf, 1000, 0, 5);
    test_one("quadratic", cpf, 10000, 0, .1);
    test_one("quadratic", cpf, 10000, 0, .5);
    test_one("quadratic", cpf, 10000, 0, 5);
  }
  return IMP::benchmark::get_return_value();
}
