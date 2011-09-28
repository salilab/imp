/**
 * Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#include <IMP.h>
#include <IMP/core.h>
#include <IMP/algebra.h>
#include <IMP/benchmark/utility.h>
#include <boost/timer.hpp>
#include <IMP/benchmark/benchmark_macros.h>
#include <IMP/container.h>

using namespace IMP;
using namespace IMP::core;
using namespace IMP::algebra;
using namespace IMP::benchmark;
using namespace IMP::container;

namespace {
  std::string get_module_name() {
    return std::string("benchmark");
  }
  std::string get_module_version() {
    return IMP::benchmark::get_module_version();
  }

class ConstPairScore: public PairScore {
public:
  ConstPairScore(){}
  IMP_INDEX_PAIR_SCORE(ConstPairScore);
};

  double ConstPairScore::evaluate_index(Model *, const ParticleIndexPair &,
                                DerivativeAccumulator *) const {
  return 1;
}
void ConstPairScore::do_show(std::ostream &) const {
}
}
ParticlesTemp ConstPairScore::get_input_particles(Particle *p) const {
  return ParticlesTemp(1,p);
}
ContainersTemp ConstPairScore::get_input_containers(Particle *) const {
  return ContainersTemp();
}

namespace {

void test_one(std::string name,
              ClosePairsFinder *cpf, unsigned int n,
              float rmin, float rmax, double) {
  set_log_level(SILENT);
  set_check_level(IMP::NONE);
  Vector3D minc(0,0,0), maxc(10,10,10);
  Model *m= new Model();
  ParticlesTemp ps = create_xyzr_particles(m, n, rmin);
  ParticleIndexes pis = IMP::internal::get_index(ps);
  ::boost::uniform_real<> rand(rmin, rmax);
  for (unsigned int i=0; i< ps.size(); ++i) {
    XYZR(ps[i]).set_radius(rand(random_number_generator));
  }
  ListSingletonContainer *lsc= new ListSingletonContainer(ps);
  ClosePairContainer *cpc= new ClosePairContainer(lsc, 0.0, cpf, 1.0);
  m->add_restraint(new PairsRestraint(new ConstPairScore(), cpc));
  double setuptime;
  IMP_TIME({
      for (unsigned int i=0; i< pis.size(); ++i) {
        XYZ(m, pis[i]).set_coordinates(get_random_vector_in(BoundingBox3D(minc,
                                                                      maxc)));
      }
    }, setuptime);
  double runtime;
  double result=0;
  IMP_TIME({
      for (unsigned int i=0; i< pis.size(); ++i) {
        XYZ(m, pis[i]).set_coordinates(get_random_vector_in(BoundingBox3D(minc,
                                                                      maxc)));
      }
      result+= m->evaluate(false);
    }, runtime);
  std::ostringstream oss;
  oss << name << " " << n << " " << rmax;
  report(oss.str(), runtime-setuptime, result);
}
}

int main() {
  {
    QuadraticClosePairsFinder *cpf= new QuadraticClosePairsFinder();
    //std::cout << "Quadratic:" << std::endl;
    test_one("col quadratic", cpf, 10000, 0, .1, 87.210356);
    test_one("col quadratic", cpf, 10000, 0, .5, 99.562332);
  }
#ifdef IMP_BENCHMARK_USE_IMP_CGAL
  {
    BoxSweepClosePairsFinder *cpf= new BoxSweepClosePairsFinder();
    //std::cout << "Box:" << std::endl;
    test_one("col box", cpf, 10000, 0, .1, 23.306047);
    test_one("col box", cpf, 10000, 0, .5, 1145.327934);
  }
#endif
  {
    GridClosePairsFinder *cpf= new GridClosePairsFinder();
    //std::cout << "Grid:" << std::endl;
    test_one("col grid", cpf, 10000, 0, .1, 23.649063);
    test_one("col grid", cpf, 10000, 0, .5, 1145.327934);
  }
  return IMP::benchmark::get_return_value();
}
