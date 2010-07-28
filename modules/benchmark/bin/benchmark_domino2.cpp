/**
 * Copyright 2007-2010 IMP Inventors. All rights reserved.
 */
#include <IMP/domino2.h>
#include <IMP/atom.h>
#include <IMP/container.h>
#include <IMP/benchmark.h>
#include <IMP/core.h>
using namespace IMP;
using namespace IMP::domino2;
using namespace IMP::atom;
using namespace IMP::algebra;
using namespace IMP::core;
using namespace IMP::container;
typedef IMP::atom::Hierarchy Hierarchy;

int main(int, char *[]) {
  IMP_NEW(Model, m, ());
  set_log_level(SILENT);
  m->set_log_level(SILENT);
  ::Hierarchy h= read_pdb(IMP::benchmark::get_data_path("small_protein.pdb"),m);
  add_radii(h);
  Transformation3Ds vs;
  HierarchiesTemp residues= get_by_type(h, RESIDUE_TYPE);
  while (residues.size() > 80) {
    residues.pop_back();
  }
  HierarchiesTemp leaves;
  for (unsigned int i=0; i< residues.size(); ++i) {
    HierarchiesTemp l= get_leaves(residues[i]);
    for (unsigned int j=0; j< l.size(); ++j) {
      leaves.push_back(l[j].get_particle());
    }
  }
  for (unsigned int i=0; i< residues.size(); ++i) {
    RigidBody r= setup_as_rigid_body(residues[i]);
    vs.push_back(r.get_transformation());
  }
  IMP_NEW(ListSingletonContainer, lsc, (leaves));
#ifdef IMP_USE_CGAL
  IMP_NEW(BoxSweepClosePairsFinder, cpf, ());
#else
  IMP_NEW(GridClosePairsFinder, cpf, ());
#endif
  cpf->set_distance(3);
  ParticlePairsTemp ppt= cpf->get_close_pairs(lsc);
  for (unsigned int i=0; i < ppt.size(); ++i) {
    if (get_residue(Atom(ppt[i][0])) == get_residue(Atom(ppt[i][1])))
      continue;
    double d= get_distance(XYZ(ppt[i][0]), XYZ(ppt[i][1]));
    Restraint *r=new DistanceRestraint(new Harmonic(d, 1),
                                       ppt[i][0], ppt[i][1]);
    std::ostringstream oss;
    oss << "Edge " << ppt[i][0]->get_name()
        << "-" << ppt[i][1]->get_name();
    r->set_name(oss.str());
    m->add_restraint(r);
  }
  IMP_NEW(RigidBodyStates, pstates, (vs));
  IMP_NEW(ParticleStatesTable, pst, ());
  for (unsigned int i=0; i< residues.size(); ++i) {
    pst->set_particle_states(residues[i], pstates);
  }
  IMP_NEW(DominoSampler, ds, (m, pst));
  ds->set_maximum_score(1);
  double runtime, num=0;
  /*#ifndef NDEBUG
  ds->set_log_level(VERBOSE);
  #endif*/
  IMP_TIME(
             {
               Pointer<ConfigurationSet> cs= ds->get_sample();
               num+=cs->get_number_of_configurations();
             }, runtime);
  IMP::benchmark::report("domino2 small ", runtime, num);
  return 0;
}
