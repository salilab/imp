/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP.h>
#include <IMP/core.h>
#include <IMP/algebra.h>
#include <IMP/benchmark/utility.h>
#include <boost/timer.hpp>
#include <IMP/benchmark/benchmark_macros.h>
#include <IMP/base/flags.h>
#include <IMP/container.h>

using namespace IMP;
using namespace IMP::core;
using namespace IMP::algebra;
using namespace IMP::benchmark;
using namespace IMP::container;

IMP_COMPILER_ENABLE_WARNINGS

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
ModelObjectsTemp
ConstPairScore::do_get_inputs(Model *m,
                              const ParticleIndexes &pis) const {
   ModelObjectsTemp ret;
   ret+=IMP::get_particles(m, pis);
   return ret;
}

namespace {

void test_one(std::string name,
              ClosePairsFinder *cpf, unsigned int n,
              float rmin, float rmax, double) {
  set_log_level(SILENT);
  set_check_level(IMP::NONE);
  Vector3D minc(0,0,0), maxc(10,10,10);
  IMP_NEW(Model, m, ());
  ParticlesTemp ps = create_xyzr_particles(m, n, rmin);
  ParticleIndexes pis = IMP::internal::get_index(ps);
  ::boost::uniform_real<> rand(rmin, rmax);
  for (unsigned int i=0; i< ps.size(); ++i) {
    XYZR(ps[i]).set_radius(rand(random_number_generator));
  }
  IMP_NEW(ListSingletonContainer, lsc, (ps));
  IMP_NEW(ClosePairContainer, cpc, (lsc, 0.0, cpf, 1.0));
  IMP_NEW(ConstPairScore, cps, ());
  IMP_NEW(PairsRestraint, pr, (cps, cpc));
  m->add_restraint(pr);
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
  oss << "col" << " " << n << " " << rmax;
  report(oss.str(), name, runtime-setuptime, result);
}
}

int main(int argc, char **argv) {
  IMP::base::setup_from_argv(argc, argv, "Benchmark collision detection");
  {
    IMP_NEW(QuadraticClosePairsFinder, cpf, ());
    //std::cout << "Quadratic:" << std::endl;
    test_one("quadratic", cpf, 10000, 0, .1, 87.210356);
    test_one("quadratic", cpf, 10000, 0, .5, 99.562332);
  }
#ifdef IMP_BENCHMARK_USE_IMP_CGAL
  {
    IMP_NEW(BoxSweepClosePairsFinder, cpf, ());
    //std::cout << "Box:" << std::endl;
    test_one("box", cpf, 10000, 0, .1, 23.306047);
    test_one("box", cpf, 10000, 0, .5, 1145.327934);
  }
#endif
  {
    IMP_NEW(GridClosePairsFinder, cpf, ());
    //std::cout << "Grid:" << std::endl;
    test_one("grid", cpf, 10000, 0, .1, 23.649063);
    test_one("grid", cpf, 10000, 0, .5, 1145.327934);
  }
  return IMP::benchmark::get_return_value();
}
