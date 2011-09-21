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
const unsigned int np=10;//5
const unsigned int nrb=10;

RigidBody create_rb(atom::Hierarchy hr) {
  Model *m=hr.get_model();
  Molecule h= Molecule::setup_particle(new Particle(m));
  XYZRs rbs;
  for (unsigned int i=0; i< np; ++i) {
    IMP_NEW(Particle, p, (m));
    p->set_name("residue g");
    Residue r= Residue::setup_particle(p, get_residue_type('G'), i);
    IMP_NEW(Particle, p1, (m));
    p1->set_name("atom");
    Atom a=Atom::setup_particle(p1, AT_CA);
    XYZR xyz= XYZR::setup_particle(p1, Sphere3D(Vector3D(8*i, 0,0), 4.0));
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


void add_excluded_volume(Model *m, atom::Hierarchy h, double k) {
  IMP_NEW(ListSingletonContainer, lsc, (atom::get_leaves(h)));
  IMP_NEW(ExcludedVolumeRestraint, evr, (lsc, k));
  evr->set_name("excluded volume");
  evr->set_log_level(VERBOSE);
  m->add_restraint(evr);
}

void add_diameter_restraint(Model *m, RigidBodies rbs, double d) {
  IMP_NEW(ListSingletonContainer, lsc, (rbs));
  IMP_NEW(HarmonicUpperBound, hub, (0,1.0));
  IMP_NEW(DiameterRestraint, dr, (hub, lsc, d));
  dr->set_name("diameter");
  m->add_restraint(dr);
}

void add_DOPE(Model *m, atom::Hierarchy h) {
  add_dope_score_data(h);
  IMP_NEW(ListSingletonContainer, lsc, (atom::get_leaves(h)));
  IMP_NEW(ClosePairContainer, cpc, (lsc, 15.0));
  IMP_NEW(DopePairScore, dps, (15.0));
  //dps->set_log_level(VERBOSE);
  IMP_NEW(PairsRestraint, dope, (dps, cpc));
  dope->set_name("DOPE");
  m->add_restraint(dope);
}

void benchmark_it(std::string name, bool incr) {
  IMP_NEW(Model, m, ());
  m->set_log_level(IMP::SILENT);
  atom::Hierarchy h= atom::Hierarchy::setup_particle(new Particle(m));
  h->set_name("root");
  RigidBodies rbs;
  for (unsigned int i=0; i < nrb; ++i) {
    rbs.push_back(create_rb(h));
  }
  add_excluded_volume(m, h, 1.0);
  add_diameter_restraint(m, rbs, 50.0);
  //add_DOPE(m, h);
  IMP_NEW(MonteCarlo, mc, (m));
  //mc->set_log_level(IMP::VERBOSE);
  mc->set_return_best(false);
  mc->set_use_incremental_evaluate(incr);
  mc->set_kt(1.0);
  Movers mvs;
  for (unsigned int i=0; i< rbs.size(); ++i) {
    IMP_NEW(RigidBodyMover, mv, (rbs[i], .5, .2));
    mvs.push_back(mv);
  }
  mc->add_mover(new SerialMover(get_as<MoversTemp>(mvs)));
  // trigger init
  mc->optimize(1);
  double runtime, score=0;
  IMP_TIME(
           score+=mc->optimize(30000), runtime);
  //std::cout << "average: "
  //<< mc->get_average_number_of_incremental_restraints() << std::endl;
  IMP::benchmark::report(name+" mc", runtime, score);
}


int main(int, char *[]) {
  benchmark_it("incremental", true);
  benchmark_it("non incremental", false);
  return 0;
}
