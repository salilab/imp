/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP.h>
#include <IMP/core.h>
#include <IMP/algebra.h>
#include <boost/timer.hpp>
#include <IMP/benchmark/utility.h>
#include <IMP/benchmark/benchmark_macros.h>
#include <IMP/core/internal/CoreClosePairContainer.h>
#include <IMP/internal/pdb.h>

using namespace IMP;
using namespace IMP::core;
using namespace IMP::algebra;

namespace {
void test_one(std::string name, Model *m, RigidBodies rbs, float side, double) {
  Vector3D minc(0, 0, 0), maxc(side, side, side);
  m->evaluate(false);
  double runtime, inittime;
  IMP_TIME({
    for (unsigned int i = 0; i < rbs.size(); ++i) {
      Vector3D t = get_random_vector_in(BoundingBox3D(minc, maxc));
      Rotation3D r = get_random_rotation_3d();
      ReferenceFrame3D tr(Transformation3D(r, t));
      rbs[i].set_reference_frame(tr);
    }
  },
           inittime);
  double value = 0;
  IMP_TIME({
    for (unsigned int i = 0; i < rbs.size(); ++i) {
      Vector3D t = get_random_vector_in(BoundingBox3D(minc, maxc));
      Rotation3D r = get_random_rotation_3d();
      Transformation3D tr(r, t);
      rbs[i].set_reference_frame(ReferenceFrame3D(tr));
    }
    value += m->evaluate(false);
  },
           runtime);

  /*std::cout << " took " << runtime-inittime << " with side " << side
    << " and value " << value << std::endl;*/
  std::ostringstream oss;
  oss << "rigid " << side;
  IMP::benchmark::report(oss.str(), name, runtime - inittime, value);
}

Model *setup(bool rpcpf, RigidBodies &rbs) {
  set_log_level(SILENT);
  set_check_level(IMP::NONE);
  Model *m = new Model();
  Particles atoms;
  for (int i = 0; i < 5; ++i) {
    std::string path = IMP::benchmark::get_data_path("small_protein.pdb");
    ParticlesTemp catoms = IMP::internal::create_particles_from_pdb(path, m);
    IMP_INTERNAL_CHECK(catoms.size() != 0, "What happened to the atoms?");
    atoms.insert(atoms.end(), catoms.begin(), catoms.end());
    IMP_NEW(Particle, rbp, (m));
    RigidBody rbd = RigidBody::setup_particle(rbp, XYZs(catoms));
    rbs.push_back(rbd);
  }
  for (unsigned int i = 0; i < atoms.size(); ++i) {
    XYZR::setup_particle(atoms[i], 1);
  }
  IMP_NEW(IMP::internal::InternalListSingletonContainer, lsc, (m, "list"));

  PairContainer *cpc;
  if (rpcpf) {
    ParticleIndexes rbsp(rbs.size());
    for (unsigned int i = 0; i < rbs.size(); ++i) {
      rbsp[i] = rbs[i].get_particle()->get_index();
    }
    lsc->set(rbsp);
    IMP_NEW(RigidClosePairsFinder, rcps, ());
    cpc = new core::internal::CoreClosePairContainer(lsc, 0.0, rcps);
  } else {
    IMP_NEW(GridClosePairsFinder, cpf, ());
    lsc->set(IMP::get_indexes(get_as<ParticlesTemp>(atoms)));
    cpc = new core::internal::CoreClosePairContainer(lsc, 0.0, cpf, 1.0);
  }
  IMP_NEW(IMP::internal::InternalPairsRestraint, pr,
          (new DistancePairScore(new Linear(1, 0)), cpc));
  m->add_restraint(pr);
  return m;
}
}

int main(int, char **) {
  {
    RigidBodies rbs;
    IMP::base::OwnerPointer<Model> m = setup(false, rbs);
    //std::cout << "Quadratic:" << std::endl;
    test_one("quadratic", m, rbs, 10, 2.578245);
    test_one("quadratic", m, rbs, 30, 1.919732);
  }
  {
    RigidBodies rbs;
    IMP::base::OwnerPointer<Model> m = setup(true, rbs);
    //std::cout << "Hierarchy:" << std::endl;
    test_one("hierarchy", m, rbs, 10, 11.549620);
    test_one("hierarchy", m, rbs, 30, 5.830277);
  }
  return IMP::benchmark::get_return_value();
}
