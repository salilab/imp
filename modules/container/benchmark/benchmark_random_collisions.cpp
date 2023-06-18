/**
 * Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#include <IMP.h>
#include <IMP/core.h>
#include <IMP/algebra.h>
#include <IMP/benchmark/utility.h>
#include <IMP/benchmark/benchmark_macros.h>
#include <IMP/flags.h>
#include <IMP/container.h>

using namespace IMP;
using namespace IMP::core;
using namespace IMP::algebra;
using namespace IMP::benchmark;
using namespace IMP::container;

IMP_COMPILER_ENABLE_WARNINGS

namespace {
std::string get_module_name() { return std::string("benchmark"); }
std::string get_module_version() {
  return IMP::benchmark::get_module_version();
}

class ConstPairScore : public PairScore {
 public:
  ConstPairScore() {}
  double evaluate_index(Model *m, const ParticleIndexPair &p,
                        DerivativeAccumulator *da) const override;
  ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const override;
  IMP_PAIR_SCORE_METHODS(ConstPairScore);
  IMP_OBJECT_METHODS(ConstPairScore);
  ;
};

double ConstPairScore::evaluate_index(Model *,
                                      const ParticleIndexPair &,
                                      DerivativeAccumulator *) const {
  return 1;
}

ModelObjectsTemp ConstPairScore::do_get_inputs(
    Model *m, const ParticleIndexes &pis) const {
  ModelObjectsTemp ret;
  ret += IMP::get_particles(m, pis);
  return ret;
}
}

namespace {

void test_one(std::string name, ClosePairsFinder *cpf, unsigned int n,
              float rmin, float rmax, double) {
  Vector3D minc(0, 0, 0), maxc(10, 10, 10);
  IMP_NEW(Model, m, ());
  ParticlesTemp ps = create_xyzr_particles(m, n, rmin);
  ParticleIndexes pis = IMP::internal::get_index(ps);
  ::boost::uniform_real<> rand(rmin, rmax);
  for (unsigned int i = 0; i < ps.size(); ++i) {
    XYZR(ps[i]).set_radius(rand(random_number_generator));
  }
  IMP_NEW(ListSingletonContainer, lsc, (m, IMP::internal::get_index(ps)));
  IMP_NEW(ClosePairContainer, cpc, (lsc, 0.0, cpf, 1.0));
  IMP_NEW(ConstPairScore, cps, ());
  IMP_NEW(PairsRestraint, pr, (cps, cpc));
  double setuptime;
  IMP_TIME({
             for (unsigned int i = 0; i < pis.size(); ++i) {
               XYZ(m, pis[i]).set_coordinates(
                   get_random_vector_in(BoundingBox3D(minc, maxc)));
             }
           },
           setuptime);
  double runtime;
  double result = 0;
  IMP_TIME({
             for (unsigned int i = 0; i < pis.size(); ++i) {
               XYZ(m, pis[i]).set_coordinates(
                   get_random_vector_in(BoundingBox3D(minc, maxc)));
             }
             result += pr->evaluate(false);
           },
           runtime);
  std::ostringstream oss;
  oss << "col"
      << " " << n << " " << rmax;
  report(oss.str(), name, runtime - setuptime, result);
}
}

int main(int argc, char **argv) {
  IMP::setup_from_argv(argc, argv, "Benchmark collision detection");
  {
    IMP_NEW(QuadraticClosePairsFinder, cpf, ());
    // std::cout << "Quadratic:" << std::endl;
    if (IMP::run_quick_test) {
      test_one("quadratic", cpf, 100, 0, .1, 87.210356);
    } else {
      test_one("quadratic", cpf, 10000, 0, .1, 87.210356);
      test_one("quadratic", cpf, 10000, 0, .5, 99.562332);
    }
  }
#ifdef IMP_BENCHMARK_USE_IMP_CGAL
  {
    IMP_NEW(BoxSweepClosePairsFinder, cpf, ());
    // std::cout << "Box:" << std::endl;
    if (IMP::run_quick_test) {
      test_one("box", cpf, 100, 0, .1, 23.306047);
    } else {
      test_one("box", cpf, 10000, 0, .1, 23.306047);
      test_one("box", cpf, 10000, 0, .5, 1145.327934);
    }
  }
#endif
  {
    IMP_NEW(GridClosePairsFinder, cpf, ());
    // std::cout << "Grid:" << std::endl;
    if (IMP::run_quick_test) {
      test_one("grid", cpf, 100, 0, .1, 23.649063);
    } else {
      test_one("grid", cpf, 10000, 0, .1, 23.649063);
      test_one("grid", cpf, 10000, 0, .5, 1145.327934);
    }
  }
  return IMP::benchmark::get_return_value();
}
