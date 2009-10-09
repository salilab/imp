/**
 * Copyright 2007-9 Sali Lab. All rights reserved.
 */

#include <IMP.h>
#include <IMP/core.h>
#include <IMP/algebra.h>
#include <IMP/benchmark/utility.h>
#include <IMP/benchmark/macros.h>

using namespace IMP;
using namespace IMP::core;
using namespace IMP::algebra;
using namespace IMP::benchmark;

void test_one(std::string name,
              ClosePairsFinder *cpf, unsigned int n,
              float rmin, float rmax) {
  set_log_level(SILENT);
  set_check_level(IMP::NONE);
  Vector3D minc(0,0,0), maxc(10,10,10);
  IMP_NEW(Model, m, ());
  Particles ps = create_xyzr_particles(m, n, rmin);
  ::boost::uniform_real<> rand(rmin, rmax);
  for (unsigned int i=0; i< ps.size(); ++i) {
    XYZ(ps[i]).set_coordinates(random_vector_in_box(minc, maxc));
    XYZR(ps[i]).set_radius(rand(random_number_generator));
  }
  IMP_NEW(ListSingletonContainer, lsc, (ps));
  IMP_NEW(ListPairContainer, out, ());
  cpf->set_distance(0);
  cpf->set_radius_key(XYZR::get_default_radius_key());
  double result=0;
  double runtime;
  IMP_TIME({
      ParticlePairsTemp ret=cpf->get_close_pairs(lsc, out);
      result+= ret.size();
      out->clear_particle_pairs();
    }, runtime);
  std::ostringstream oss;
  oss << name << " cpf " << n << " " << rmax;
  report(oss.str(), runtime, result);
}

int main() {
  {
    IMP_NEW(QuadraticClosePairsFinder, cpf, ());
    //std::cout << "Quadratic:" << std::endl;
    test_one("quadratic", cpf, 1000, 0, .1);
    test_one("quadratic", cpf, 1000, 0, .5);
    test_one("quadratic", cpf, 10000, 0, .1);
    test_one("quadratic", cpf, 10000, 0, .5);
  }
#ifdef IMP_USE_CGAL
  {
    IMP_NEW(BoxSweepClosePairsFinder, cpf, ());
    //std::cout << "Box:" << std::endl;
    test_one("box", cpf, 1000, 0, .1);
    test_one("box", cpf, 1000, 0, .5);
    test_one("box", cpf, 10000, 0, .1);
    test_one("box", cpf, 100000, 0, .01);
  }
#endif
  {
    IMP_NEW(GridClosePairsFinder, cpf, ());
    //std::cout << "Grid:" << std::endl;
    test_one("grid", cpf, 1000, 0, .1);
    test_one("grid", cpf, 1000, 0, .5);
    test_one("grid", cpf, 10000, 0, .1);
    test_one("grid", cpf, 100000, 0, .01);
  }
  return 0;
}
