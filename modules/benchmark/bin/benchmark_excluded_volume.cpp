/**
 * Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#include <IMP.h>
#include <IMP/core.h>
#include <IMP/algebra.h>
#include <IMP/atom.h>
#include <IMP/container.h>
#include <IMP/benchmark/utility.h>
#include <IMP/benchmark/benchmark_macros.h>
#include <IMP/container.h>

using namespace IMP;
using namespace IMP::core;
using namespace IMP::algebra;
using namespace IMP::benchmark;
using namespace IMP::container;
using namespace IMP::atom;
using namespace IMP::container;

/** One bringing slowly together and one jumping around randomly, with
    all pairs, pair container, evr and evaluate if good or not for each
*/

namespace {
#if IMP_BUILD==IMP_FAST
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
              Model *m, XYZ to_move,
              bool eig,
              int argc, char *argv[]) {
  set_log_level(SILENT);
  set_check_level(IMP::NONE);

  Restraints rs= get_restraints(m->get_root_restraint_set());
  Floats weights(rs.size(), 1);

  IMP::algebra::BoundingBox3D bb
    = IMP::algebra::BoundingBox3D(IMP::algebra::Vector3D(-100,-100,-100),
                                  IMP::algebra::Vector3D( 100, 100, 100));
  unsigned int nreps=onreps;
  if (argc>=4) {
    nreps*=1000;
  }
  if (argc<4 || argv[3][0]=='r'){
    double result=0;
    double runtime;
    IMP_TIME({
        for (unsigned int i=0; i< nreps; ++i) {
          to_move.set_coordinates(IMP::algebra::get_random_vector_in(bb));
          if (eig) {
            result+=get_val(m->evaluate_if_good(rs, weights, false)[0]);
          } else {
            result+=get_val(m->evaluate(false));
          }
        }
      }, runtime);
    std::ostringstream oss;
    oss << name << " random"<< (eig?" if good":"");
    report(oss.str(), runtime, result);
  }
  if (argc<4 || argv[3][0]=='s') {
    to_move.set_coordinates(IMP::algebra::Vector3D(0,0,0));
    double result=0;
    double runtime;
    IMP_TIME({
        for (unsigned int i=0; i< nreps; ++i) {
          to_move.set_x(100.0*static_cast<double>(i)/nreps);
          if (eig) {
            result+=get_val(m->evaluate_if_good(rs,
                                           weights, false)[0]);
          } else {
            result+=get_val(m->evaluate(false));
          }
        }
      }, runtime);
    std::ostringstream oss;
    oss << name << " systematic"<< (eig?" if good":"");
    report(oss.str(), runtime, result);
  }
  if (argc<4 || argv[3][0]=='f') {
    IMP::algebra::Sphere3D s(IMP::algebra::Vector3D(0,0,0), 60);
    double result=0;
    double runtime;
    IMP_TIME({
        for (unsigned int i=0; i< nreps; ++i) {
          to_move.set_coordinates(IMP::algebra::get_random_vector_on(s));
          if (eig) {
            result+=get_val(m->evaluate_if_good(rs, weights, false)[0]);
          } else {
            result+=get_val(m->evaluate(false));
          }
        }
      }, runtime);
    std::ostringstream oss;
    oss << name << " far"<< (eig?" if good":"");
    report(oss.str(), runtime, result);
  }
  if (argc<4 || argv[3][0]=='c') {
    IMP::algebra::Sphere3D s(IMP::algebra::Vector3D(0,0,0), 4);
    double result=0;
    double runtime;
    IMP_TIME({
        for (unsigned int i=0; i< nreps; ++i) {
          to_move.set_coordinates(IMP::algebra::get_random_vector_on(s));
          if (eig) {
            result+=get_val(m->evaluate_if_good(rs, weights, false)[0]);
          } else {
            result+=get_val(m->evaluate(false));
          }
        }
      }, runtime);
    std::ostringstream oss;
    oss << name << " close"<< (eig?" if good":"");
    report(oss.str(), runtime, result);
  }
}
}

int main(int argc, char *argv[]) {
  IMP_NEW(Model, m, ());
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
  std::cout << leaves.size() << " particles" << std::endl;
  if (argc==1 || (argc >1 && argv[1][0]=='k')) {
    IMP_NEW(SoftSpherePairScore, ps, (1));
    IMP_NEW(TableRefiner, tr,());
    tr->add_particle(rb0, get_leaves(h0));
    tr->add_particle(rb1, get_leaves(h1));
    IMP_NEW(KClosePairsPairScore, cpps, (ps, tr, 1));
    ScopedRestraint sr(create_restraint(cpps.get(),
                                        ParticlePair(rb0, rb1)),
                       m->get_root_restraint_set());
    sr->set_maximum_score(.1);
    if (argc<3 || argv[2][0]=='b') {
      test_one<KClosePairsPairScore>("k close", m, rb0, false, argc, argv);
    }
    if (argc<3 || argv[2][0]=='g') {
      test_one<KClosePairsPairScore>("k close", m, rb0, true, argc, argv);
    }
  }
  if (argc==1 || (argc >1 && argv[1][0]=='e')) {

    IMP_NEW(ExcludedVolumeRestraint, evr, (lsc,1, 5));
    evr->set_maximum_score(.1);
    ScopedRestraint sr(evr.get(), m->get_root_restraint_set());
    if (argc<3 || argv[2][0]=='b') {
      test_one<ExcludedVolumeRestraint>("excluded volume", m, rb0,
                                        false, argc, argv);
    }
    if (argc<3 || argv[2][0]=='g') {
      test_one<ExcludedVolumeRestraint>("excluded volume", m, rb0,
                                        true, argc, argv);
    }
  }
  if (argc==1 || (argc >1 && argv[1][0]=='p')) {
    IMP_NEW(ClosePairContainer, cpc, (lsc, 0, 5));
    IMP_NEW(SoftSpherePairScore, ps, (1));
    ScopedRestraint sr(create_restraint(ps.get(), cpc.get()),
                       m->get_root_restraint_set());
    sr->set_maximum_score(.1);
    if (argc<3 || argv[2][0]=='b') {
      test_one<ClosePairContainer>("pairs restraint", m, rb0,
                                   false, argc, argv);
    }
    if (argc<3 || argv[2][0]=='g') {
      test_one<ClosePairContainer>("pairs restraint", m, rb0,
                                   true, argc, argv);
    }
  }

  return IMP::benchmark::get_return_value();
}
