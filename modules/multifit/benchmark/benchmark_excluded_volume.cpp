/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

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
  const unsigned int onreps=400;
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
              Model *m, ScoringFunction *sf,
              XYZ to_move,
              bool eig) {
  RestraintsTemp rs
    = get_restraints(RestraintsTemp(1, m->get_root_restraint_set()));
  Floats weights(rs.size(), 1);

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

void do_it() {
 IMP_NEW(Model, m, ());
 int seed = IMP::base::random_number_generator();
  atom::Hierarchy h0
    = read_pdb(IMP::benchmark::get_data_path("small_protein.pdb"), m);
  atom::Hierarchy h1
    = read_pdb(IMP::benchmark::get_data_path("small_protein.pdb"), m);
  RigidBody rb0= create_rigid_body(h0);
  RigidBody rb1= create_rigid_body(h1);
  rb0.set_coordinates(IMP::algebra::Vector3D(0,0,0));
  rb1.set_coordinates(IMP::algebra::Vector3D(0,0,0));
  //std::cout << core::XYZR(rb0).get_radius() << std::endl;
  ParticlesTemp leaves= get_leaves(h0);
  ParticlesTemp leaves1= get_leaves(h1);
  leaves.insert(leaves.end(), leaves1.begin(), leaves1.end());
  IMP_NEW(ListSingletonContainer, lsc, (leaves));
  lsc->set_was_used(true);
  //std::cout << leaves.size() << " particles" << std::endl;
  {
    IMP_NEW(SoftSpherePairScore, ps, (1));
    IMP_NEW(TableRefiner, tr,());
    tr->add_particle(rb0, get_leaves(h0));
    tr->add_particle(rb1, get_leaves(h1));
    IMP_NEW(ClosePairsPairScore, cpps, (ps, tr, 0));
    Pointer<Restraint> sr(IMP::create_restraint(cpps.get(),
                                        ParticlePair(rb0, rb1)));
    sr->set_maximum_score(.1);
    test_one<ClosePairsPairScore>("close pair score", seed, m,
                                   sr->create_scoring_function(),
                                   rb0, false);
    test_one<ClosePairsPairScore>("close pair score", seed, m,
                                   sr->create_scoring_function(),
                                   rb0, true);
  }
  {

    IMP_NEW(ExcludedVolumeRestraint, evr, (lsc,1, 5));
    evr->set_maximum_score(.1);
    test_one<ExcludedVolumeRestraint>("excluded volume", seed, m,
                                      evr->create_scoring_function(), rb0,
                                      false);
    test_one<ExcludedVolumeRestraint>("excluded volume", seed, m,
                                      evr->create_scoring_function(), rb0,
                                      true);
  }
  {
    IMP_NEW(ClosePairContainer, cpc, (lsc, 0, 5));
    IMP_NEW(SoftSpherePairScore, ps, (1));
    Pointer<Restraint> sr(container::create_restraint(ps.get(), cpc.get()));
    sr->set_maximum_score(.1);
    test_one<ClosePairContainer>("pairs restraint", seed, m,
                                 sr->create_scoring_function(), rb0,
                                 false);
    test_one<ClosePairContainer>("pairs restraint", seed, m,
                                 sr->create_scoring_function(), rb0,
                                 true);
  }
  {
    IMP_NEW(ComplementarityRestraint, r, (atom::get_leaves(h0),
                                                  atom::get_leaves(h1)));
    r->set_maximum_penetration_score(10);
    r->set_maximum_separation(10);
    r->set_complementarity_value(0);
    r->set_complementarity_thickness(0);
    r->set_interior_layer_thickness(4);
    test_one<ComplementarityRestraint>("wev3", seed, m,
                                       r->create_scoring_function(), rb0,
                                       false);
    test_one<ComplementarityRestraint>("wev3", seed, m,
                                       r->create_scoring_function(), rb0,
                                       true);
  }
}
}

int main(int argc, char *argv[]) {
  IMP::base::setup_from_argv(argc, argv,
                             "Benchmark methods for excluded volume");
  IMP_CATCH_AND_TERMINATE(do_it(););
  return IMP::benchmark::get_return_value();
}
