/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPMULTIFIT_BENCHMARK_EXCLUDED_VOLUME_H
#define IMPMULTIFIT_BENCHMARK_EXCLUDED_VOLUME_H

#include <IMP/core/ExcludedVolumeRestraint.h>
#include <IMP/container/generic.h>
#include <IMP/container/ClosePairContainer.h>
#include <IMP/Model.h>
#include <IMP/benchmark/utility.h>
#include <IMP/benchmark/benchmark_macros.h>
#include <IMP/base/flags.h>
#include <IMP/atom/pdb.h>
#include <IMP/algebra/vector_generators.h>
#include <IMP/core/TableRefiner.h>
#include <IMP/core/ClosePairsPairScore.h>
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/multifit/ComplementarityRestraint.h>

using namespace IMP;
using namespace IMP::core;
using namespace IMP::algebra;
using namespace IMP::benchmark;
using namespace IMP::container;
using namespace IMP::atom;
using namespace IMP::container;
using namespace IMP::multifit;

/** One bringing slowly together and one jumping around randomly, with
    all pairs, pair container, evr and evaluate if good or not for each
*/

namespace {

#if IMP_BUILD>=IMP_RELEASE
  const unsigned int onreps=200;
#else
  const unsigned int onreps=2;
#endif
  double get_val(double v) {
    if (v>.1) return 0;
    else return 1;
  }

  template <class Tag>
void test_one(std::string name,
              int seed,
              Model *,
              ScoringFunction *sf,
              XYZ to_move,
              bool eig) {
  // Take ownership of ScoringFunction object and make sure it's refcounted
  base::PointerMember<ScoringFunction> osf = sf;
  IMP::algebra::BoundingBox3D bb
    = IMP::algebra::BoundingBox3D(IMP::algebra::Vector3D(-100,-100,-100),
                                  IMP::algebra::Vector3D( 100, 100, 100));
  unsigned int nreps=onreps;

  {
    double result=0, total_reps=0;
    double runtime;
    IMP_TIME({
        IMP::base::random_number_generator.seed(seed);
        for (unsigned int i=0; i< nreps; ++i) {
          to_move.set_coordinates(IMP::algebra::get_random_vector_in(bb));
          if (eig) {
            result+=get_val(sf->evaluate_if_good(false));
          } else {
            result+=get_val(sf->evaluate(false));
          }
          ++total_reps;
        }
      }, runtime);
    std::ostringstream oss;
    oss << "random"<< (eig?" if good":"");
    report(name, oss.str(), runtime, result/total_reps);
  }
  {
    to_move.set_coordinates(IMP::algebra::Vector3D(0,0,0));
    double result=0, total_reps=0;;
    double runtime;
    IMP_TIME({
        IMP::base::random_number_generator.seed(seed);
        for (unsigned int i=0; i< nreps; ++i) {
          to_move.set_x(100.0*static_cast<double>(i)/nreps);
          if (eig) {
            result+=get_val(sf->evaluate_if_good(false));
          } else {
            result+=get_val(sf->evaluate(false));
          }
          ++total_reps;
        }
      }, runtime);
    std::ostringstream oss;
    oss <<"systematic"<< (eig?" if good":"");
    report(name, oss.str(), runtime, result/total_reps);
  }
  {
    IMP::algebra::Sphere3D s(IMP::algebra::Vector3D(0,0,0), 60);
    double result=0, total_reps=0;;
    double runtime;
    IMP_TIME({
        IMP::base::random_number_generator.seed(seed);
        for (unsigned int i=0; i< nreps; ++i) {
          to_move.set_coordinates(IMP::algebra::get_random_vector_on(s));
          if (eig) {
            result+=get_val(sf->evaluate_if_good(false));
          } else {
            result+=get_val(sf->evaluate(false));
          }
          ++total_reps;
        }
      }, runtime);
    std::ostringstream oss;
    oss << "far"<< (eig?" if good":"");
    report(name, oss.str(), runtime, result/total_reps);
  }
  {
    IMP::algebra::Sphere3D s(IMP::algebra::Vector3D(0,0,0), 4);
    double result=0, total_reps=0;;
    double runtime;
    IMP_TIME({
        IMP::base::random_number_generator.seed(seed);
        for (unsigned int i=0; i< nreps; ++i) {
          to_move.set_coordinates(IMP::algebra::get_random_vector_on(s));
          if (eig) {
            result+=get_val(sf->evaluate_if_good(false));
          } else {
            result+=get_val(sf->evaluate(false));
          }
          ++total_reps;
        }
      }, runtime);
    std::ostringstream oss;
    oss << "close"<< (eig?" if good":"");
    report(name, oss.str(), runtime, result/total_reps);
  }
}

}

#define IMP_EV_BENCHMARK_SETUP  \
 IMP_NEW(Model, m, ()); \
 int seed = IMP::base::random_number_generator(); \
  atom::Hierarchy h0 \
    = read_pdb(IMP::benchmark::get_data_path("small_protein.pdb"), m); \
  atom::Hierarchy h1 \
    = read_pdb(IMP::benchmark::get_data_path("small_protein.pdb"), m); \
  RigidBody rb0= create_rigid_body(h0); \
  RigidBody rb1= create_rigid_body(h1); \
  rb0.set_coordinates(IMP::algebra::Vector3D(0,0,0)); \
  rb1.set_coordinates(IMP::algebra::Vector3D(0,0,0)); \
  ParticlesTemp leaves= get_leaves(h0); \
  ParticlesTemp leaves1= get_leaves(h1); \
  leaves.insert(leaves.end(), leaves1.begin(), leaves1.end()); \
  IMP_NEW(ListSingletonContainer, lsc, (leaves)); \
  lsc->set_was_used(true);

#endif  /* IMPMULTIFIT_BENCHMARK_EXCLUDED_VOLUME_H */
