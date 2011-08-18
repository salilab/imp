/**
 * Copyright 2007-2011 IMP Inventors. All rights reserved.
 */
#include <IMP/core.h>
#include <IMP/atom.h>
#include <IMP/container.h>
#include <IMP/benchmark.h>
#include <IMP.h>
using namespace IMP;
using namespace IMP::core;
using namespace IMP::atom;
using namespace IMP::container;
using namespace IMP::algebra;

RigidBody create_rb(atom::Hierarchy hr) {
  Model *m=hr.get_model();
  Molecule h= Molecule::setup_particle(new Particle(m));
  XYZRs rbs;
  for (unsigned int i=0; i< 5; ++i) {
    IMP_NEW(Particle, p, (m));
    Residue r= Residue::setup_particle(p, get_residue_type('G'), i);
    IMP_NEW(Particle, p1, (m));
    Atom a=Atom::setup_particle(p1, AT_CA);
    XYZR xyz= XYZR::setup_particle(p1, Sphere3D(Vector3D(8*i, 0,0), 4.0));
    r.add_child(a);
    rbs.push_back(xyz);
    h.add_child(r);
  }
  hr.add_child(h);
  IMP_NEW(Particle, prb, (m));
  RigidBody rb= RigidBody::setup_particle(prb, rbs);
  return rb;
}


void add_excluded_volume(Model *m, atom::Hierarchy h, double k) {
  IMP_NEW(ListSingletonContainer, lsc, (atom::get_leaves(h)));
  IMP_NEW(ExcludedVolumeRestraint, evr, (lsc, k));
  m->add_restraint(evr);
}

void add_diameter_restraint(Model *m, RigidBodies rbs, double d) {
  IMP_NEW(ListSingletonContainer, lsc, (rbs));
  IMP_NEW(HarmonicUpperBound, hub, (0,1.0));
  IMP_NEW(DiameterRestraint, dr, (hub, lsc, d));
  m->add_restraint(dr);
}

void add_DOPE(Model *m, atom::Hierarchy h) {
  add_dope_score_data(h);
  IMP_NEW(ListSingletonContainer, lsc, (atom::get_leaves(h)));
  IMP_NEW(ClosePairContainer, cpc, (lsc, 15.0));
  IMP_NEW(DopePairScore, dps, (15));
  IMP_NEW(PairsRestraint, dope, (dps, cpc));
  dope->set_name("DOPE");
  m->add_restraint(dope);
}

void benchmark_it(std::string name, bool incr) {
  IMP_NEW(Model, m, ());
  m->set_log_level(IMP::SILENT);
  atom::Hierarchy h= atom::Hierarchy::setup_particle(new Particle(m));
  int nrbs=5;
  RigidBodies rbs;
  for ( int i=0; i < nrbs; ++i) {
    rbs.push_back(create_rb(h));
  }
  add_excluded_volume(m, h, 1.0);
  add_diameter_restraint(m, rbs, 50.0);
  add_DOPE(m, h);
  IMP_NEW(MonteCarlo, mc, (m));
  mc->set_return_best(false);
  mc->set_use_incremental_evaluate(incr);
  mc->set_kt(1.0);
  Movers mvs;
  for (unsigned int i=0; i< rbs.size(); ++i) {
    IMP_NEW(RigidBodyMover, mv, (rbs[i], .5, .2));
    mvs.push_back(mv);
  }
  mc->add_mover(new SerialMover(mvs));
  double runtime, score=0;
  IMP_TIME(
           score+=mc->optimize(500), runtime);
  IMP::benchmark::report(name+" mc", runtime, score);
}


int main(int, char *[]) {
  benchmark_it("non incremental", false);
  benchmark_it("incremental", true);
  return 0;
}
