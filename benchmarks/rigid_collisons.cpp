/**
 * Copyright 2007-9 Sali Lab. All rights reserved.
 */

#include <IMP.h>
#include <IMP/core.h>
#include <IMP/algebra.h>
#include <IMP/atom.h>
#include <boost/timer.hpp>

using namespace IMP;
using namespace IMP::core;
using namespace IMP::algebra;
using namespace IMP::atom;

void test_one(Model *m,
              std::vector<RigidBodyDecorator> rbs,
              ListSingletonContainer *lsc,
              float side) {
  Vector3D minc(0,0,0), maxc(side, side, side);
  set_log_level(SILENT);
  m->evaluate(false);
  double runtime, inittime;
  IMP_TIME(
           {
             for (unsigned int i=0; i< rbs.size(); ++i) {
               Vector3D t= random_vector_in_box(minc, maxc);
               Rotation3D r= random_rotation();
               Transformation3D tr(r, t);
               rbs[i].set_transformation(tr, true);
             }
           }, inittime);
  IMP_TIME(
           {
             for (unsigned int i=0; i< rbs.size(); ++i) {
               Vector3D t= random_vector_in_box(minc, maxc);
               Rotation3D r= random_rotation();
               Transformation3D tr(r, t);
               rbs[i].set_transformation(tr, true);
             }
             m->evaluate(false);
           }, runtime);

  std::cout << " took " << runtime-inittime << " with side " << side
            << std::endl;
}

int main() {
  Pointer<Model> m(new Model());
  Particles atoms;
  std::vector<RigidBodyDecorator> rbs;
  for (int i=0; i< 10; ++i) {
    MolecularHierarchyDecorator mhd
      = read_pdb("benchmarks/input/single_protein.pdb", m);
    Particles catoms= get_by_type(mhd, MolecularHierarchyDecorator::ATOM);
    atoms.insert(atoms.end(), catoms.begin(), catoms.begin());
    ScoreState *ss= create_rigid_body(mhd.get_particle(),
                                       catoms);
    m->add_score_state(ss);
    rbs.push_back(RigidBodyDecorator(mhd.get_particle()));
    set_enclosing_sphere(XYZRDecorator::create(mhd.get_particle()),
                         catoms);
  }
  IMP_NEW(lsc, ListSingletonContainer, ());
  IMP_NEW(cpss, ClosePairsScoreState, (lsc));
  m->add_score_state(cpss);
  IMP_NEW(pr, PairsRestraint,
          (new DistancePairScore(new Harmonic(0,1)),
           cpss->get_close_pairs_container()));
  m->add_restraint(pr);

  {
    IMP_NEW(qcpf, BoxSweepClosePairsFinder, ());
    cpss->set_close_pairs_finder(qcpf);
    lsc->set_particles(atoms);
    std::cout << "Box:" << std::endl;
    test_one(m, rbs, lsc, 10);
    test_one(m, rbs, lsc, 100);
    test_one(m, rbs, lsc, 1000);
  }
  {
    IMP_NEW(qcpf, QuadraticClosePairsFinder, ());
    lsc->set_particles(atoms);
    cpss->set_close_pairs_finder(qcpf);
    std::cout << "Quadratic:" << std::endl;
    test_one(m, rbs, lsc, 10);
    test_one(m, rbs, lsc, 100);
    test_one(m, rbs, lsc, 1000);
  }
  {
    Particles rbsp(rbs.size());
    for (unsigned int i=0; i< rbs.size(); ++i){
      rbsp[i]= rbs[i].get_particle();
    }
    lsc->set_particles(rbsp);
    IMP_NEW(rcps, RigidClosePairScore, (pr->get_pair_score(), 0));
    pr->set_pair_score(rcps);
    std::cout << "Hierarchy:" << std::endl;
    test_one(m, rbs, lsc, 10);
    test_one(m, rbs, lsc, 100);
    test_one(m, rbs, lsc, 1000);
  }
  return 0;
}
