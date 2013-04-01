/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */
#include <IMP/core.h>
#include <IMP/atom.h>
#include <IMP/base/flags.h>
#include <IMP/container.h>
#include <IMP/benchmark.h>
#include <IMP.h>
using namespace IMP;
using namespace IMP::core;
using namespace IMP::atom;
using namespace IMP::container;
using namespace IMP::algebra;

namespace {
const unsigned int np=10;//5
const unsigned int nrb=10;
const double radius=4;

RigidBody create_rb(atom::Hierarchy hr) {
  Model *m=hr.get_model();
  Molecule h= Molecule::setup_particle(new Particle(m));
  XYZRs rbs;
  Sphere3D last(Vector3D(0,0,0), radius);
  for (unsigned int i=0; i< np; ++i) {
    IMP_NEW(Particle, p, (m));
    p->set_name("residue g");
    Residue r= Residue::setup_particle(p, get_residue_type('G'), i);
    IMP_NEW(Particle, p1, (m));
    p1->set_name("atom");
    Atom a=Atom::setup_particle(p1, AT_CA);
    XYZR xyz= XYZR::setup_particle(p1, Sphere3D(get_random_vector_on(last),
                                                radius));
    last=xyz.get_sphere();
    r.add_child(a);
    rbs.push_back(xyz);
    h.add_child(r);
  }
  hr.add_child(h);
  IMP_NEW(Particle, prb, (m));
  prb->set_name(h->get_name() + " rb");
  RigidBody rb= RigidBody::setup_particle(prb, rbs);
  return rb;
}


Restraint* create_excluded_volume(atom::Hierarchy h, RigidBodies,
                                  double k) {
    IMP_NEW(ListSingletonContainer, lsc, (atom::get_leaves(h)));
    IMP_NEW(ExcludedVolumeRestraint, evr, (lsc, k, 10));
    evr->set_name("excluded volume");
    //evr->set_log_level(VERBOSE);
    return evr.release();
}
PairScore *create_pair_score(atom::Hierarchy , RigidBodies rbs,
                             double k) {
  IMP_NEW(core::SoftSpherePairScore, ssps, (k));
  IMP_NEW(core::TableRefiner, ref, ());
  for (unsigned int i=0; i< rbs.size(); ++i) {
    ref->add_particle(rbs[i], rbs[i].get_members());
  }
  IMP_NEW(core::ClosePairsPairScore, cpps, (ssps, ref, 0));
  return cpps.release();
}

Restraint* create_diameter_restraint(Model *, RigidBodies rbs, double d) {
  IMP_NEW(ListSingletonContainer, lsc, (rbs));
  IMP_NEW(HarmonicUpperBound, hub, (0,1.0));
  IMP_NEW(DiameterRestraint, dr, (hub, lsc, d));
  dr->set_name("diameter");
  return dr.release();
}

Restraint* add_DOPE(Model *, atom::Hierarchy h) {
  add_dope_score_data(h);
  IMP_NEW(ListSingletonContainer, lsc, (atom::get_leaves(h)));
  IMP_NEW(ClosePairContainer, cpc, (lsc, 15.0));
  IMP_NEW(DopePairScore, dps, (15.0));
  //dps->set_log_level(VERBOSE);
  IMP_NEW(PairsRestraint, dope, (dps, cpc));
  dope->set_name("DOPE");
  return dope.release();
}

void benchmark_it(std::string name, bool incr, bool nbl, bool longr) {
  IMP_NEW(Model, m, ());
  m->set_log_level(IMP::SILENT);
  set_check_level(IMP::USAGE_AND_INTERNAL);
  atom::Hierarchy h= atom::Hierarchy::setup_particle(new Particle(m));
  h->set_name("root");
  RigidBodies rbs;
  for (unsigned int i=0; i < nrb; ++i) {
    rbs.push_back(create_rb(h));
    rbs.back().set_coordinates(algebra::Vector3D(0,1000*i,0));
  }
  IMP_NEW(MonteCarlo, mc, (m));
  Restraints rs;
  if (!incr && nbl) {
    rs.push_back(create_excluded_volume(h, rbs, 1.0));
  }
  rs.push_back(create_diameter_restraint(m, rbs, 50.0));
  if (incr) {
    IMP_NEW(core::IncrementalScoringFunction, sf, (rbs, rs));
    if (nbl) {
      sf->add_close_pair_score(create_pair_score(h, rbs, 1.0), 0,
                               rbs);
    }
    mc->set_incremental_scoring_function(sf);
  } else {
    IMP_NEW(core::RestraintsScoringFunction, sf, (rs));
    mc->set_scoring_function(sf);
  }
  //add_DOPE(m, h);
  //mc->set_log_level(VERBOSE);
  mc->set_return_best(false);
  mc->set_kt(1.0);
  MonteCarloMovers mvs;
  for (unsigned int i=0; i< rbs.size(); ++i) {
    IMP_NEW(RigidBodyMover, mv, (rbs[i], 80, .2));
    mvs.push_back(mv);
  }
  mc->add_mover(new SerialMover(get_as<MonteCarloMovers>(mvs)));
  // trigger init
  mc->optimize(1);
#if IMP_BUILD == IMP_DEBUG
  unsigned int nsteps=300;
#else
  unsigned int nsteps=30000;
#endif
  if (longr) nsteps*=100;
  double runtime, score=0;
  IMP_TIME(
           score+=mc->optimize(nsteps), runtime);
  //std::cout << "average: "
  //<< mc->get_average_number_of_incremental_restraints() << std::endl;
  IMP::benchmark::report(name+" mc", runtime, score);
}
}


int main(int argc, char *argv[]) {
  IMP::base::setup_from_argv(argc, argv, "Benchmark incremenal evaluation");
  benchmark_it("incremental nbl", true, true, argc>1);
  benchmark_it("non incremental", false, false, false);
  benchmark_it("incremental", true, false, argc>1);
  return 0;
}
