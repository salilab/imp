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
  ::Hierarchy h= read_pdb(IMP::benchmark::get_data_path("small_protein.pdb"),m);
  add_radii(h);
  Vector3Ds vs;
  HierarchiesTemp leaves= get_leaves(h);
  for (unsigned int i=0; i< leaves.size(); ++i) {
    vs.push_back(XYZ(leaves[i]).get_coordinates());
  }
  IMP_NEW(ListSingletonContainer, lsc, (leaves));
  IMP_NEW(BoxSweepClosePairsFinder, cpf, ());
  cpf->set_distance(5);
  ParticlePairsTemp ppt= cpf->get_close_pairs(lsc);
  for (unsigned int i=0; i < ppt.size(); ++i) {
    double d= get_distance(XYZ(ppt[i][0]), XYZ(ppt[i][1]));
    m->add_restraint(new DistanceRestraint(new Harmonic(d, 1),
                                           ppt[i][0], ppt[i][1]));
  }
  IMP_NEW(XYZsStates, pstates, (vs));
  IMP_NEW(DominoSampler, ds, (m));
  for (unsigned int i=0; i< leaves.size(); ++i) {
    ds->get_particle_states_table()->set_particle_states(leaves[i], pstates);
  }
  ds->set_maximum_score(1);
  double runtime, num=0;
  IMP_TIME(
             {
               Pointer<ConfigurationSet> cs= ds->get_sample();
               num+=cs->get_number_of_configurations();
             }, runtime);
  IMP::benchmark::report("domino2 small ", runtime, num);
  return 0;
}
