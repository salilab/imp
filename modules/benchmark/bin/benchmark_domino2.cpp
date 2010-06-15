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
  m->set_log_level(SILENT);
  ::Hierarchy h= read_pdb(IMP::benchmark::get_data_path("small_protein.pdb"),m);
  add_radii(h);
  Transformation3Ds vs;
  HierarchiesTemp residues= get_by_type(h, RESIDUE_TYPE);
  HierarchiesTemp leaves= get_leaves(h);
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
    double d= get_distance(XYZ(ppt[i][0]), XYZ(ppt[i][1]));
    m->add_restraint(new DistanceRestraint(new Harmonic(d, 1),
                                           ppt[i][0], ppt[i][1]));
  }
  IMP_NEW(RigidBodyStates, pstates, (vs));
  IMP_NEW(ParticleStatesTable, pst, ());
  for (unsigned int i=0; i< residues.size(); ++i) {
    pst->set_particle_states(residues[i], pstates);
  }
  IMP_NEW(CachingModelSubsetEvaluatorTable, mset, (m, pst));
  IMP_NEW(DefaultSubsetStatesTable, dsst, (pst));
  dsst->set_subset_evaluator_table(mset);
  IMP_NEW(DominoSampler, ds, (m, pst));
  ds->set_subset_states_table(dsst);
  ds->set_subset_evaluator_table(mset);

  ds->set_maximum_score(1);
  double runtime, num=0;
  m->set_log_level(SILENT);
  ds->set_log_level(VERBOSE);
  IMP_TIME(
             {
               Pointer<ConfigurationSet> cs= ds->get_sample();
               num+=cs->get_number_of_configurations();
             }, runtime);
  IMP::benchmark::report("domino2 small ", runtime, num);
  return 0;
}
