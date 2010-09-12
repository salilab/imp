/**
 * Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#include <IMP.h>
#include <IMP/core.h>
#include <IMP/algebra.h>
#include <IMP/atom.h>
#include <IMP/helper.h>
#include <boost/timer.hpp>
#include <IMP/benchmark/utility.h>
#include <IMP/benchmark/benchmark_macros.h>
#include <IMP/container.h>

using namespace IMP;
using namespace IMP::core;
using namespace IMP::algebra;
using namespace IMP::atom;
using namespace IMP::helper;
using namespace IMP::container;

void test_one(std::string name,
              Model *m,
              RigidBodiesTemp rbs,
              float side,
              double target) {
  VectorD<3> minc(0,0,0), maxc(side, side, side);
  m->evaluate(false);
  double runtime, inittime;
  IMP_TIME(
           {
             for (unsigned int i=0; i< rbs.size(); ++i) {
               VectorD<3> t= get_random_vector_in(BoundingBox3D(minc, maxc));
               Rotation3D r= get_random_rotation_3d();
               ReferenceFrame3D tr(Transformation3D(r, t));
               rbs[i].set_reference_frame(tr);
             }
           }, inittime);
  double value=0;
  IMP_TIME(
           {
             for (unsigned int i=0; i< rbs.size(); ++i) {
               VectorD<3> t= get_random_vector_in(BoundingBox3D(minc, maxc));
               Rotation3D r= get_random_rotation_3d();
               Transformation3D tr(r, t);
               rbs[i].set_reference_frame(ReferenceFrame3D(tr));
             }
             value+=m->evaluate(false);
           }, runtime);

  /*std::cout << " took " << runtime-inittime << " with side " << side
    << " and value " << value << std::endl;*/
  std::ostringstream oss;
  oss << "rigid " << name << " " << side;
  IMP::benchmark::report(oss.str(), runtime-inittime, target, value);
}


Model * setup(bool rpcpf,RigidBodiesTemp &rbs) {
  set_log_level(SILENT);
  set_check_level(IMP::NONE);
  Model *m=new Model();
  Particles atoms;
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
  IMP_NEW(ListSingletonContainer, lsc, (m));

  PairContainer *cpc;
  if (rpcpf) {
    Particles rbsp(rbs.size());
    for (unsigned int i=0; i< rbs.size(); ++i){
      rbsp.set(i, rbs[i].get_particle());
    }
    lsc->set_particles(rbsp);
    IMP_NEW(RigidClosePairsFinder, rcps,
            (new LeavesRefiner(atom::Hierarchy::get_traits())));
    cpc= new ClosePairContainer(lsc, 0.0, rcps);
  } else {
    lsc->set_particles(atoms);
    cpc = new ClosePairContainer(lsc, 0.0);
  }
  IMP_NEW(PairsRestraint, pr,
          (new DistancePairScore(new Linear(1,0)),
           cpc));
  m->add_restraint(pr);
  return m;
}

int main() {
  {
    RigidBodiesTemp rbs;
    IMP::internal::OwnerPointer<Model> m
      = setup(false, rbs);
    //std::cout << "Quadratic:" << std::endl;
    test_one("quadratic", m, rbs, 10, 2.578245);
    test_one("quadratic", m, rbs, 30, 1.919732);
  }
  {
    RigidBodiesTemp rbs;
    IMP::internal::OwnerPointer<Model> m
      = setup(true, rbs);
    //std::cout << "Hierarchy:" << std::endl;
    test_one("hierarchy", m, rbs, 10, 11.549620);
    test_one("hierarchy", m, rbs, 30, 5.830277);
  }
  return IMP::benchmark::get_return_value();
}
