/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */
#include <IMP.h>
#include <IMP/core.h>
#include <IMP/algebra.h>
#include <boost/timer.hpp>
#include <IMP/benchmark/utility.h>
#include <IMP/benchmark/benchmark_macros.h>
#include <IMP/base/flags.h>
#include <IMP/base/thread_macros.h>

IMP_COMPILER_ENABLE_WARNINGS

namespace {
  std::string get_module_name() {
    return "benchmark omp";
  }
  std::string get_module_version() {
    return IMP::core::get_module_version();
  }

  class ExpensiveRestraint: public IMP::Restraint {
    IMP::ParticleIndexes pis_;
  public:
    ExpensiveRestraint(IMP::Model *m, const IMP::ParticleIndexes &pis):
      Restraint(m, "ExpensiveRestraint%1%"), pis_(pis) {}

    IMP_RESTRAINT_ACCUMULATOR(ExpensiveRestraint);
  };
  void ExpensiveRestraint::do_show(std::ostream &) const {}
  void ExpensiveRestraint
  ::do_add_score_and_derivatives(IMP::ScoreAccumulator sa) const {
    double score=0;
    IMP::Model *m= get_model();
    for (unsigned int i=0; i< pis_.size(); ++i) {
      IMP::core::XYZ di(m, pis_[i]);
      IMP::algebra::Vector3D vi=di.get_coordinates();
      double cur_score=0.0;
      for (unsigned int j=0; j< pis_.size(); ++j) {
        IMP::algebra::Vector3D vj=IMP::core::XYZ(m, pis_[j]).get_coordinates();
        double cs= IMP::algebra::get_distance(vi, vj);
        cur_score+=cs;
      }
      if (sa.get_derivative_accumulator()) {
        di.add_to_derivatives(IMP::algebra::get_ones_vector_d<3>()*cur_score,
                              *sa.get_derivative_accumulator());
      }
      score+=cur_score;
    }
    sa.add_score(score);
  }

  IMP::ModelObjectsTemp ExpensiveRestraint::do_get_inputs() const {
    return IMP::get_particles(get_model(), pis_);
  }

  void benchmark_omp(IMP::core::RestraintsScoringFunction *sf) {
    std::ostringstream oss;
    oss << IMP::base::get_number_of_threads();

    IMP_THREADS((oss),
                {
                  double timet; double score=0.0;
                  IMP_WALLTIME({
                      score=sf->evaluate(false);
                    }, timet);
                  IMP::benchmark::report(std::string("omp evaluate no deriv ")
                                         +oss.str(), timet, score);
                });
    IMP_THREADS((oss),
                {
                  double time; double score=0.0;
                  IMP_WALLTIME({
                      score=sf->evaluate(true);
                    }, time);
                  IMP::benchmark::report(std::string("omp evaluate deriv ")
                                         +oss.str(), time, score);
                });
  }

  void benchmark_serial(IMP::core::RestraintsScoringFunction *sf) {
    {
      double time, score=0.0;
      IMP_WALLTIME({
          score=sf->evaluate(false);
        }, time);
      IMP::benchmark::report("serial evaluate no deriv", time, score);
    }
    {
      double time, score=0.0;
      IMP_WALLTIME({
          score=sf->evaluate(true);
        }, time);
      IMP::benchmark::report("serial evaluate deriv", time, score);
    }
  }

}

int main(int argc, char **argv) {
  IMP::base::setup_from_argv(argc, argv, "Benchmark OpenMP evaluations");
  IMP::algebra::BoundingBox3D bb= IMP::algebra::get_unit_bounding_box_d<3>();
  IMP_NEW(IMP::Model, m, ());
  IMP::Restraints rs;
  for (unsigned int i=0; i < 15; ++i) {
    IMP::ParticleIndexes pis;
    for (unsigned int j=0; j< 5000; ++j) {
      IMP_NEW(IMP::Particle, p, (m));
      IMP::core::XYZ::setup_particle(p, IMP::algebra::get_random_vector_in(bb));
      pis.push_back(p->get_index());
    }
    IMP_NEW(ExpensiveRestraint, er, (m, pis));
    rs.push_back(er);
  }
  IMP_NEW(IMP::core::RestraintsScoringFunction, sf, (rs));
  // to update dependency graph and all
  sf->evaluate(false);
  benchmark_omp(sf);
  benchmark_serial(sf);
  return IMP::benchmark::get_return_value();
}
