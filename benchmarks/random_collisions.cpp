/**
 * Copyright 2007-9 Sali Lab. All rights reserved.
 */

#include <IMP.h>
#include <IMP/core.h>
#include <IMP/algebra.h>
#include <boost/timer.hpp>

using namespace IMP;
using namespace IMP::core;
using namespace IMP::algebra;

void test_one(ClosePairsFinder *cpf, unsigned int n, float rmin, float rmax) {
  Vector3D minc(0,0,0), maxc(10,10,10);
  Model *m= new Model();
  Particles ps = create_xyzr_particles(m, n, rmin);
  ::boost::uniform_real<> rand(rmin, rmax);
  for (unsigned int i=0; i< ps.size(); ++i) {
    XYZRDecorator(ps[i]).set_radius(rand(random_number_generator));
  }
  ListSingletonContainer *lsc= new ListSingletonContainer(ps);
  ClosePairsScoreState *cpss= new ClosePairsScoreState(lsc);
  cpss->set_close_pairs_finder(cpf);

  set_check_level(NONE);
  set_log_level(SILENT);
  double setuptime;
  IMP_TIME({
      for (unsigned int i=0; i< ps.size(); ++i) {
        XYZDecorator(ps[i]).set_coordinates(random_vector_in_box(minc, maxc));
      }
    }, setuptime);
  double runtime;
  IMP_TIME({
      for (unsigned int i=0; i< ps.size(); ++i) {
        XYZDecorator(ps[i]).set_coordinates(random_vector_in_box(minc, maxc));
      }
      cpss->before_evaluate();
    }, runtime);
  std::cout << "Collision detection on " << n << " particles with radii from "
            << rmin << " to " << rmax
            << " took " << runtime-setuptime
            << std::endl;
}

int main() {
  {
    QuadraticClosePairsFinder *cpf= new QuadraticClosePairsFinder();
    std::cout << "Quadratic:" << std::endl;
    test_one(cpf, 10, 0, 1);
    test_one(cpf, 100, 0, 1);
    test_one(cpf, 1000, 0, 1);
    test_one(cpf, 10000, 0, .5);
    test_one(cpf, 10000, 0, .1);
  }
  {
    BoxSweepClosePairsFinder *cpf= new BoxSweepClosePairsFinder();
    std::cout << "Box:" << std::endl;
    test_one(cpf, 10, 0, 1);
    test_one(cpf, 100, 0, 1);
    test_one(cpf, 1000, 0, 1);
    test_one(cpf, 10000, 0, .5);
    test_one(cpf, 10000, 0, .1);
  }
  {
    GridClosePairsFinder *cpf= new GridClosePairsFinder();
    std::cout << "Grid:" << std::endl;
    test_one(cpf, 10, 0, 1);
    test_one(cpf, 100, 0, 1);
    test_one(cpf, 1000, 0, 1);
    test_one(cpf, 10000, 0, .5);
    test_one(cpf, 10000, 0, .1);
  }
  return 0;
}
