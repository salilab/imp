/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */
#include <IMP/core/SphereDistancePairScore.h>
#include <IMP/Particle.h>
#include <boost/timer.hpp>
#include <IMP/benchmark/utility.h>
#include <IMP/internal/AccumulatorScoreModifier.h>
#include <IMP/benchmark/benchmark_macros.h>
#include <IMP/base/flags.h>
#include <IMP/container/PairContainerSet.h>
#include <IMP/container/ListPairContainer.h>
using namespace IMP;
using namespace IMP::base;
using namespace IMP::core;
using namespace IMP::algebra;
using namespace IMP::container;

namespace {

#define IMP_GET_EVALUATE(Class)\
  static_cast<double (Class::*)(Model *, const ParticleIndexPair&,      \
                                DerivativeAccumulator*)                 \
              const>(&Class::evaluate_index)

template <class It, class F>
inline double apply_and_accumulate(It b, It e, F f) {
  double ret=0;
  for (It c=b; c != e; ++c) {
    ret+= f(*c);
  }
  return ret;
}


void time_both(PairContainer *pc, PairScore *ps, std::string name) {
  std::ostringstream ossc;
  ossc << "container " << pc->get_indexes().size();
  {
    const ParticleIndexPairs pps= pc->get_indexes();
    double runtime=0, total=0;
    IMP_TIME(
             {
               for (unsigned int i=0; i< pps.size(); ++i) {
                 total+=ps->evaluate_index(pc->get_model(), pps[i], nullptr);
               }
             }, runtime);
    std::ostringstream oss;
    oss << "direct " << name;
    IMP::benchmark::report(ossc.str(), oss.str(), runtime, total);
  }
  {
    SoftSpherePairScore *ssps= dynamic_cast<SoftSpherePairScore*>(ps);
    const ParticleIndexPairs pps= pc->get_indexes();
    double runtime=0, total=0;
    IMP_TIME(
             {
               for (unsigned int i=0; i< pps.size(); ++i) {
                 total+=ssps->evaluate_index(pc->get_model(), pps[i], nullptr);
               }
             }, runtime);
    std::ostringstream oss;
    oss << "ssps direct " << name;
    IMP::benchmark::report(ossc.str(), oss.str(), runtime, total);
  }
  {
    SoftSpherePairScore *ssps= dynamic_cast<SoftSpherePairScore*>(ps);
    const ParticleIndexPairs pps= pc->get_indexes();
    double runtime=0, total=0;
    IMP_TIME(
             {
               for (unsigned int i=0; i< pps.size(); ++i) {
                 total+=
                   ssps->SoftSpherePairScore::evaluate_index(pc->get_model(),
                                                             pps[i],
                                                             nullptr);
               }
             }, runtime);
    std::ostringstream oss;
    oss << "ssps direct call " << name;
    IMP::benchmark::report(ossc.str(), oss.str(), runtime, total);
  }
  {
    SoftSpherePairScore *ssps= dynamic_cast<SoftSpherePairScore*>(ps);
    const ParticleIndexPairs pps= pc->get_indexes();
    double runtime=0, total=0;
    IMP_TIME(
             {
               total+=apply_and_accumulate(pps.begin(), pps.end(),
                             boost::bind(IMP_GET_EVALUATE(SoftSpherePairScore),
                                         ssps, pc->get_model(), _1,
                                         nullptr));
             }, runtime);
    std::ostringstream oss;
    oss << "ssps direct bind " << name;
    IMP::benchmark::report(ossc.str(), oss.str(), runtime, total);
  }
  {
    SoftSpherePairScore *ssps= dynamic_cast<SoftSpherePairScore*>(ps);
    const ParticleIndexPairs pps= pc->get_indexes();
    double runtime=0, total=0;
    IMP_TIME(
             {
               total+=apply_and_accumulate(pps.begin(), pps.end(),
                                 boost::bind(IMP_GET_EVALUATE(PairScore),
                                             ssps, pc->get_model(), _1,
                         static_cast<DerivativeAccumulator*>(nullptr)));
             }, runtime);
    std::ostringstream oss;
    oss << "direct bind " << name;
    IMP::benchmark::report(ossc.str(), oss.str(), runtime, total);
  }
  /*{
    double runtime=0, total=0;
    Pointer<IMP::internal::AccumulatorScoreModifier<PairScore> >
        am= IMP::internal::create_accumulator_score_modifier(ps);

    IMP_TIME(
             {
               pc->apply_generic(am);
               total=am->get_score();
             }, runtime);
    std::ostringstream oss;
    oss << name;
    IMP::benchmark::report(ossc.str(), oss.str(), runtime, total);
    }*/
  {
   double runtime=0, total=0;
    IMP_TIME(
             {
               IMP_CONTAINER_FOREACH(PairContainer, pc,
                                     {
                 total+= ps->evaluate_index(pc->get_model(),
                                            _1, nullptr);
                                     });
             }, runtime);
    std::ostringstream oss;
    oss << name << " out";
    IMP::benchmark::report(ossc.str(), oss.str(), runtime, total);
  }
}

void test(int n) {
  set_log_level(IMP::base::SILENT);
  IMP_NEW(Model, m, ());
  ParticlesTemp ps= create_xyzr_particles(m, n, .1);
  IMP_NEW(ListPairContainer, lpc, (m));
  for (unsigned int i=0; i< ps.size(); ++i) {
    for (unsigned int j=0; j< i; ++j) {
      lpc->add_particle_pair(ParticlePair(ps[i], ps[j]));
    }
  }
  IMP_NEW(SoftSpherePairScore, dps, (1));
  time_both(lpc, dps, "list");
}

void test_set(int n) {
  set_log_level(SILENT);
  IMP_NEW(Model, m, ());
  ParticlesTemp ps= create_xyzr_particles(m, n, .1);
  IMP_NEW(ListPairContainer, lpc0, (m));
  for (unsigned int i=0; i< ps.size()/2; ++i) {
    for (unsigned int j=0; j< i; ++j) {
      lpc0->add_particle_pair(ParticlePair(ps[i], ps[j]));
    }
  }

  IMP_NEW(ListPairContainer, lpc1, (m));
  for (unsigned int i=ps.size()/2; i< ps.size(); ++i) {
    for (unsigned int j=ps.size()/2; j< i; ++j) {
      lpc1->add_particle_pair(ParticlePair(ps[i], ps[j]));
    }
  }
  IMP_NEW(PairContainerSet, pcs, (m));
  pcs->add_pair_container(lpc0);
  pcs->add_pair_container(lpc1);

  IMP_NEW(SoftSpherePairScore, dps, (1));
  time_both(pcs, dps, "set");
}
}


int main(int argc, char **argv) {
  IMP::base::setup_from_argv(argc, argv, "Benchmark evaluation");
  {
    test(100);
  }
  {
    test_set(100);
  }
  return IMP::benchmark::get_return_value();
}
