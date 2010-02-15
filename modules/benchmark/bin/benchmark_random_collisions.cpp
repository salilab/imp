/**
 * Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#include <IMP.h>
#include <IMP/core.h>
#include <IMP/algebra.h>
#include <IMP/benchmark/utility.h>
#include <boost/timer.hpp>
#include <IMP/benchmark/macros.h>
#include <IMP/container.h>

using namespace IMP;
using namespace IMP::core;
using namespace IMP::algebra;
using namespace IMP::benchmark;
using namespace IMP::container;

class ConstPairScore: public PairScore {
public:
  ConstPairScore(){}
  IMP_SIMPLE_PAIR_SCORE(ConstPairScore, VersionInfo());
};
double ConstPairScore::evaluate(const ParticlePair &,
                                DerivativeAccumulator *) const {
  return 1;
}
void ConstPairScore::show(std::ostream &out) const {
  out << "Hi";
}

void test_one(std::string name,
              ClosePairsFinder *cpf, unsigned int n,
              float rmin, float rmax) {
  set_log_level(SILENT);
  set_check_level(IMP::NONE);
  Vector3D minc(0,0,0), maxc(10,10,10);
  Model *m= new Model();
  Particles ps = create_xyzr_particles(m, n, rmin);
  ::boost::uniform_real<> rand(rmin, rmax);
  for (unsigned int i=0; i< ps.size(); ++i) {
    XYZR(ps[i]).set_radius(rand(random_number_generator));
  }
  ListSingletonContainer *lsc= new ListSingletonContainer(ps);
  ClosePairContainer *cpc= new ClosePairContainer(lsc, 0.0, cpf, 1.0);
  m->add_restraint(new PairsRestraint(new ConstPairScore(), cpc));
  double setuptime;
  IMP_TIME({
      for (unsigned int i=0; i< ps.size(); ++i) {
        XYZ(ps[i]).set_coordinates(random_vector_in_box(minc, maxc));
      }
    }, setuptime);
  double runtime;
  double result=0;
  IMP_TIME({
      for (unsigned int i=0; i< ps.size(); ++i) {
        XYZ(ps[i]).set_coordinates(random_vector_in_box(minc, maxc));
      }
      result+= m->evaluate(false);
    }, runtime);
  std::ostringstream oss;
  oss << name << " " << n << " " << rmax;
  report(oss.str(), runtime-setuptime, result);
}

int main() {
  {
    QuadraticClosePairsFinder *cpf= new QuadraticClosePairsFinder();
    //std::cout << "Quadratic:" << std::endl;
    test_one("col quadratic", cpf, 10000, 0, .1);
    test_one("col quadratic", cpf, 10000, 0, .5);
  }
#ifdef IMP_USE_CGAL
  {
    BoxSweepClosePairsFinder *cpf= new BoxSweepClosePairsFinder();
    //std::cout << "Box:" << std::endl;
    test_one("col box", cpf, 10000, 0, .1);
    test_one("col box", cpf, 100000, 0, .01);
  }
#endif
  {
    GridClosePairsFinder *cpf= new GridClosePairsFinder();
    //std::cout << "Grid:" << std::endl;
    test_one("col grid", cpf, 10000, 0, .1);
    test_one("col grid", cpf, 100000, 0, .01);
  }
  return 0;
}
