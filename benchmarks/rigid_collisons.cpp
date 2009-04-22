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
              float side) {
  Vector3D minc(0,0,0), maxc(side, side, side);
  set_log_level(SILENT);
  m->evaluate(false);
  set_log_level(SILENT);
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
  double value;
  IMP_TIME(
           {
             for (unsigned int i=0; i< rbs.size(); ++i) {
               Vector3D t= random_vector_in_box(minc, maxc);
               Rotation3D r= random_rotation();
               Transformation3D tr(r, t);
               rbs[i].set_transformation(tr, true);
             }
             value=m->evaluate(false);
           }, runtime);

  std::cout << " took " << runtime-inittime << " with side " << side
            << " and value " << value << std::endl;
}

int main() {
  Pointer<Model> m(new Model());
  Particles atoms;
  std::vector<RigidBodyDecorator> rbs;
  for (int i=0; i< 5; ++i) {
    MolecularHierarchyDecorator mhd
      = read_pdb("benchmarks/input/single_protein.pdb", m);
    Particles catoms= get_by_type(mhd, MolecularHierarchyDecorator::ATOM);
    IMP_assert(catoms.size() != 0, "What happened to the atoms?");
    atoms.insert(atoms.end(), catoms.begin(), catoms.end());
    ScoreState *ss= create_rigid_body(mhd.get_particle(),
                                       catoms);
    m->add_score_state(ss);
    rbs.push_back(RigidBodyDecorator(mhd.get_particle()));
    set_enclosing_sphere(XYZRDecorator::create(mhd.get_particle()),
                         catoms);
  }
  for (unsigned int i=0; i< atoms.size(); ++i) {
    XYZRDecorator::create(atoms[i], 1);
  }
  IMP_NEW(lsc, ListSingletonContainer, (atoms));
  IMP_NEW(cpss, ClosePairsScoreState, (lsc));
  m->add_score_state(cpss);
  IMP_NEW(pr, PairsRestraint,
          (new DistancePairScore(new Linear(1,0)),
           cpss->get_close_pairs_container()));
  m->add_restraint(pr);
  {
    IMP_NEW(qcpf, QuadraticClosePairsFinder, ());
    //lsc->set_particles(atoms);
    cpss->set_close_pairs_finder(qcpf);
    std::cout << "Quadratic:" << std::endl;
    test_one(m, rbs, 10);

    test_one(m, rbs, 100);

    test_one(m, rbs, 1000);

  }
  {
    Particles rbsp(rbs.size());
    for (unsigned int i=0; i< rbs.size(); ++i){
      rbsp[i]= rbs[i].get_particle();
    }
    lsc->set_particles(rbsp);
    IMP_NEW(rcps, RigidClosePairsFinder, ());
    cpss->set_close_pairs_finder(rcps);
    std::cout << "Hierarchy:" << std::endl;
    test_one(m, rbs, 10);

    test_one(m, rbs, 100);

    test_one(m, rbs, 1000);

  }
  return 0;
}
