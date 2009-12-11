/**
 * Copyright 2007-9 Sali Lab. All rights reserved.
 */

#include <IMP.h>
#include <IMP/core.h>
#include <IMP/algebra.h>
#include <IMP/atom.h>
#include <IMP/helper.h>
#include <boost/timer.hpp>
#include <IMP/benchmark/utility.h>
#include <IMP/benchmark/macros.h>

using namespace IMP;
using namespace IMP::core;
using namespace IMP::algebra;
using namespace IMP::atom;
using namespace IMP::helper;

void test_one(std::string name,
              Model *m,
              std::vector<RigidBody> rbs,
              float side) {
  Vector3D minc(0,0,0), maxc(side, side, side);
  m->evaluate(false);
  double runtime, inittime;
  IMP_TIME(
           {
             for (unsigned int i=0; i< rbs.size(); ++i) {
               Vector3D t= random_vector_in_box(minc, maxc);
               Rotation3D r= random_rotation();
               Transformation3D tr(r, t);
               rbs[i].set_transformation(tr);
             }
           }, inittime);
  double value=0;
  IMP_TIME(
           {
             for (unsigned int i=0; i< rbs.size(); ++i) {
               Vector3D t= random_vector_in_box(minc, maxc);
               Rotation3D r= random_rotation();
               Transformation3D tr(r, t);
               rbs[i].set_transformation(tr);
             }
             value+=m->evaluate(false);
           }, runtime);

  /*std::cout << " took " << runtime-inittime << " with side " << side
    << " and value " << value << std::endl;*/
  std::ostringstream oss;
  oss << "rigid " << name << " " << side;
  IMP::benchmark::report(oss.str(), runtime-inittime, value);
}

int main() {
  set_log_level(SILENT);
  set_check_level(IMP::NONE);
  IMP::internal::OwnerPointer<Model> m(new Model());
  Particles atoms;
  std::vector<RigidBody> rbs;
  for (int i=0; i< 5; ++i) {
    atom::Hierarchy mhd
      = read_pdb(IMP::benchmark::get_data_path("small_protein.pdb"), m);
    Particles catoms= get_by_type(mhd, atom::ATOM_TYPE);
    IMP_INTERNAL_CHECK(catoms.size() != 0, "What happened to the atoms?");
    atoms.insert(atoms.end(), catoms.begin(), catoms.end());
    RigidBody rbd=RigidBody::setup_particle(mhd.get_particle(),
                                            XYZs(catoms));
    rbs.push_back(rbd);
  }
  for (unsigned int i=0; i< atoms.size(); ++i) {
    XYZR::setup_particle(atoms[i], 1);
  }
  IMP_NEW(ListSingletonContainer, lsc, (atoms));
  IMP_NEW(ClosePairsScoreState, cpss, (lsc));
  m->add_score_state(cpss);
  IMP_NEW(PairsRestraint, pr,
          (new DistancePairScore(new Linear(1,0)),
           cpss->get_close_pairs_container()));
  m->add_restraint(pr);
  {
    IMP_NEW(QuadraticClosePairsFinder,qcpf, ());
    //lsc->set_particles(atoms);
    cpss->set_close_pairs_finder(qcpf);
    //std::cout << "Quadratic:" << std::endl;
    test_one("quadratic", m, rbs, 100);
    test_one("quadratic", m, rbs, 1000);

  }
  {
    Particles rbsp(rbs.size());
    for (unsigned int i=0; i< rbs.size(); ++i){
      rbsp.set(i, rbs[i].get_particle());
    }
    lsc->set_particles(rbsp);
    IMP_NEW(RigidClosePairsFinder, rcps, ());
    cpss->set_close_pairs_finder(rcps);
    //std::cout << "Hierarchy:" << std::endl;
    test_one("hierarchy", m, rbs, 100);
    test_one("hierarchy", m, rbs, 1000);

  }
  return 0;
}
