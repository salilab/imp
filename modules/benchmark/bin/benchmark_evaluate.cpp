/**
 * Copyright 2007-2010 IMP Inventors. All rights reserved.
 */
#include <IMP.h>
#include <IMP/core.h>
#include <IMP/algebra.h>
#include <IMP/atom.h>
#include <IMP/container.h>
#include <boost/timer.hpp>
#include <IMP/benchmark/utility.h>
#include <IMP/benchmark/benchmark_macros.h>

using namespace IMP;
using namespace IMP::core;
using namespace IMP::algebra;
using namespace IMP::atom;
using namespace IMP::container;

#define IMP_GET_EVALUATE(Class)\
  static_cast<double (Class::*)(const ParticlePair&,                    \
                                DerivativeAccumulator*)                 \
              const>(&Class::evaluate)

template <class It, class F>
inline double my_accumulate(It b, It e, F f) {
  double ret=0;
  for (It c=b; c != e; ++c) {
    ret+= f(*c);
  }
  return ret;
}

void time_both(PairContainer *pc, PairScore *ps, std::string name,
               double *targets) {
  {
    const ParticlePairsTemp pps= pc->get_particle_pairs();
    double runtime=0, total=0;
    IMP_TIME(
             {
               for (unsigned int i=0; i< pps.size(); ++i) {
                 total+=ps->evaluate(pps[i], NULL);
               }
             }, runtime);
    std::ostringstream oss;
    oss << "container direct " << name << " in "
        << pc->get_number_of_particle_pairs();
    IMP::benchmark::report(oss.str(), runtime, targets[0], total);
  }
  {
    SoftSpherePairScore *ssps= dynamic_cast<SoftSpherePairScore*>(ps);
    const ParticlePairsTemp pps= pc->get_particle_pairs();
    double runtime=0, total=0;
    IMP_TIME(
             {
               for (unsigned int i=0; i< pps.size(); ++i) {
                 total+=ssps->evaluate(pps[i], NULL);
               }
             }, runtime);
    std::ostringstream oss;
    oss << "container ssps direct " << name << " in "
        << pc->get_number_of_particle_pairs();
    IMP::benchmark::report(oss.str(), runtime, targets[1], total);
  }
  {
    SoftSpherePairScore *ssps= dynamic_cast<SoftSpherePairScore*>(ps);
    const ParticlePairsTemp pps= pc->get_particle_pairs();
    double runtime=0, total=0;
    IMP_TIME(
             {
               for (unsigned int i=0; i< pps.size(); ++i) {
                 total+=ssps->SoftSpherePairScore::evaluate(pps[i],
                                   static_cast<DerivativeAccumulator*>(NULL));
               }
             }, runtime);
    std::ostringstream oss;
    oss << "container ssps direct call " << name << " in "
        << pc->get_number_of_particle_pairs();
    IMP::benchmark::report(oss.str(), runtime, targets[2], total);
  }
  {
    SoftSpherePairScore *ssps= dynamic_cast<SoftSpherePairScore*>(ps);
    const ParticlePairsTemp pps= pc->get_particle_pairs();
    double runtime=0, total=0;
    IMP_TIME(
             {
               total+=my_accumulate(pps.begin(), pps.end(),
                             boost::bind(IMP_GET_EVALUATE(SoftSpherePairScore),
                                             ssps, _1,
                                    static_cast<DerivativeAccumulator*>(NULL)));
             }, runtime);
    std::ostringstream oss;
    oss << "container ssps direct bind " << name << " in "
        << pc->get_number_of_particle_pairs();
    IMP::benchmark::report(oss.str(), runtime, targets[3], total);
  }
  {
    SoftSpherePairScore *ssps= dynamic_cast<SoftSpherePairScore*>(ps);
    const ParticlePairsTemp pps= pc->get_particle_pairs();
    double runtime=0, total=0;
    IMP_TIME(
             {
               total+=my_accumulate(pps.begin(), pps.end(),
                                 boost::bind(IMP_GET_EVALUATE(PairScore),
                                             ssps, _1,
                            static_cast<DerivativeAccumulator*>(NULL)));
             }, runtime);
    std::ostringstream oss;
    oss << "container direct bind " << name << " in "
        << pc->get_number_of_particle_pairs();
    IMP::benchmark::report(oss.str(), runtime, targets[4], total);
  }
  {
    double runtime=0, total=0;
    IMP_TIME(
             {
               total+=pc->evaluate(ps, NULL);
             }, runtime);
    std::ostringstream oss;
    oss << "container " << name << " in "
        << pc->get_number_of_particle_pairs();
    IMP::benchmark::report(oss.str(), runtime, targets[5], total);
  }
  {
   double runtime=0, total=0;
    IMP_TIME(
             {
               for (unsigned int i=0; i< pc->get_number_of_particle_pairs();
                    ++i) {
                 total+= ps->evaluate(pc->get_particle_pair(i), NULL);
               }
             }, runtime);
    std::ostringstream oss;
    oss << "container " << name << " out "
        << pc->get_number_of_particle_pairs();
    IMP::benchmark::report(oss.str(), runtime, targets[6], total);
  }
}

void test(int n, double *targets) {
  set_log_level(SILENT);
  IMP_NEW(Model, m, ());
  Particles ps= create_xyzr_particles(m, n, .1);
  IMP_NEW(ListPairContainer, lpc, (m));
  for (unsigned int i=0; i< ps.size(); ++i) {
    for (unsigned int j=0; j< i; ++j) {
      lpc->add_particle_pair(ParticlePair(ps[i], ps[j]));
    }
  }
  IMP_NEW(SoftSpherePairScore, dps, (1));
  time_both(lpc, dps, "list", targets);
}

void test_set(int n, double *targets) {
  set_log_level(SILENT);
  IMP_NEW(Model, m, ());
  Particles ps= create_xyzr_particles(m, n, .1);
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
  time_both(pcs, dps, "set", targets);
}



int main(int , char **) {
  {
    double targets[]={0.005960, 0.005930, 0.006022, 0.005563,
                      0.005571, 0.005869, 0.007210};
    test(100, targets);
  }
  {
    double targets[]={0.634483, 0.630137, 0.636678, 0.595469,
                      0.593548, 0.619529, 0.773109};
    test(1000, targets);
  }
  {
    double targets[]={0.002984, 0.002973, 0.003014, 0.002807,
                      0.002793, 0.002918, 0.006555};
    test_set(100, targets);
  }
  {
    double targets[]={0.308208, 0.306156, 0.310287, 0.288401,
                      0.286159, 0.300654, 0.669091};
    test_set(1000, targets);
  }
  return IMP::benchmark::get_return_value();
}
