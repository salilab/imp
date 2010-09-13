/**
 * Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#include <IMP.h>
#include <IMP/core.h>
#include <IMP/algebra.h>
#include <IMP/benchmark/utility.h>
#include <IMP/benchmark/benchmark_macros.h>
#include <IMP/container.h>

using namespace IMP;
using namespace IMP::core;
using namespace IMP::algebra;
using namespace IMP::benchmark;
using namespace IMP::container;

void test_one(std::string name,
              ClosePairsFinder *cpf, unsigned int n,
              float rmin, float rmax,
              // assume by and non-bi are close enough for now
              double target,
              bool nobi=false) {
  set_log_level(SILENT);
  set_check_level(IMP::NONE);
  {
    VectorD<3> minc(0,0,0), maxc(10,10,10);
    IMP_NEW(Model, m, ());
    Particles ps = create_xyzr_particles(m, n, rmin);
    ::boost::uniform_real<> rand(rmin, rmax);
    for (unsigned int i=0; i< ps.size(); ++i) {
      XYZ(ps[i])
        .set_coordinates(get_random_vector_in(BoundingBox3D(minc, maxc)));
      XYZR(ps[i]).set_radius(rand(random_number_generator));
    }
    IMP_NEW(ListSingletonContainer, lsc, (ps));
    cpf->set_distance(0);
    double result=0;
    double runtime;
    IMP_TIME({
        result+=cpf->get_close_pairs(lsc).size();
      }, runtime);
    std::ostringstream oss;
    oss << "cpf " << name << " " << n << " " << rmax;
    report(oss.str(), runtime, target, result);
  }
  if (!nobi) {
    VectorD<3> minc(0,0,0), maxc(10,10,10);
    IMP_NEW(Model, m, ());
    Particles ps0 = create_xyzr_particles(m, n, rmin);
    Particles ps1 = create_xyzr_particles(m, n, rmin);
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
    IMP_NEW(ListSingletonContainer, lsc0, (ps0));
    IMP_NEW(ListSingletonContainer, lsc1, (ps1));
    cpf->set_distance(0);
    double result=0;
    double runtime;
    IMP_TIME({
        result+=cpf->get_close_pairs(lsc0, lsc1).size();
      }, runtime);
    std::ostringstream oss;
    oss << "bcpf " << name << " " << n << " " << rmax;
    report(oss.str(), runtime, target, result);
  }
}

int main() {
  {
    IMP_NEW(GridClosePairsFinder, cpf, ());
#if IMP_BUILD == IMP_DEBUG
    cpf->set_log_level(IMP::VERBOSE);
#endif
    std::string name="grid";
    test_one(name, cpf, 1000, 0, .1, 0.097368, true);
    test_one(name, cpf, 1000, 0, .5, 0.372233, true);
    test_one(name, cpf, 1000, 0, 5, 1.697248, true);
    test_one(name, cpf, 10000, 0, .1, 1.156250, true);
    test_one(name, cpf, 10000, 0, .5, 4.353953, true);
    test_one(name, cpf, 10000, 0, 5, 141.340000, true);
    //test_one(name, cpf, 100000, 0, .01, 18.648000, true);
    //test_one(name, cpf, 100000, 0, .1, 23.217500, true);
    //test_one(name, cpf, 100000, 0, .3, 51.800000, true);
  }
  {
    IMP_NEW(NearestNeighborsClosePairsFinder, cpf, ());
    std::string name="nn";
    test_one(name, cpf, 1000, 0, .1, 0.247989);
    test_one(name, cpf, 1000, 0, .5, 0.478036);
    test_one(name, cpf, 1000, 0, 5, 5.628485);
    test_one(name, cpf, 10000, 0, .1, 3.869583);
    test_one(name, cpf, 10000, 0, .5, 13.848571);
    //test_one(name, cpf, 10000, 0, 5);
    //test_one(name, cpf, 100000, 0, .01, 42.624000);
    //test_one(name, cpf, 100000, 0, .1, 85.593333);
    //test_one(name, cpf, 100000, 0, .3, 329.300000);
  }
#ifdef IMP_USE_CGAL
  {
    IMP_NEW(BoxSweepClosePairsFinder, cpf, ());
    std::string name="box";
    // bi takes twice as long as non-bi
    test_one(name, cpf, 1000, 0, .1, 0.103064);
    test_one(name, cpf, 1000, 0, .5, 0.287714);
    test_one(name, cpf, 1000, 0, 5, 2.681159);
    test_one(name, cpf, 10000, 0, .1, 3.805714);
    test_one(name, cpf, 10000, 0, .5, 9.324000);
    test_one(name, cpf, 10000, 0, 5, 190.920000);
    //test_one(name, cpf, 100000, 0, .01, 44.696000);
    //test_one(name, cpf, 100000, 0, .1, 95.830000);
    //test_one(name, cpf, 100000, 0, .3, 198.320000);
  }
#endif
  {
    IMP_NEW(QuadraticClosePairsFinder, cpf, ());
    //std::cout << "Quadratic:" << std::endl;
    // bi also twice as as slow
    test_one("quadratic", cpf, 1000, 0, .1, 0.653710);
    test_one("quadratic", cpf, 1000, 0, .5, 0.790598);
    test_one("quadratic", cpf, 1000, 0, 5, 2.636901);
    test_one("quadratic", cpf, 10000, 0, .1, 65.613333);
    test_one("quadratic", cpf, 10000, 0, .5, 79.673333);
    test_one("quadratic", cpf, 10000, 0, 5, 300.440000);
  }
  return IMP::benchmark::get_return_value();
}
