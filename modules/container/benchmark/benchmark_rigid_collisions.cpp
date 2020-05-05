/**
 * Copyright 2007-2020 IMP Inventors. All rights reserved.
 */

#include <IMP.h>
#include <IMP/core.h>
#include <IMP/algebra.h>
#include <IMP/flags.h>
#include <boost/timer.hpp>
#include <IMP/benchmark/utility.h>
#include <IMP/benchmark/benchmark_macros.h>
#include <IMP/container/internal/ClosePairContainer.h>
#include <IMP/internal/pdb.h>

using namespace IMP;
using namespace IMP::core;
using namespace IMP::algebra;

namespace {
void test_one(std::string name, Restraint *r, RigidBodies rbs,
              float side, double) {
  Vector3D minc(0, 0, 0), maxc(side, side, side);
  r->evaluate(false);
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
             value += r->evaluate(false);
           },
           runtime);

  /*std::cout << " took " << runtime-inittime << " with side " << side
    << " and value " << value << std::endl;*/
  std::ostringstream oss;
  oss << "rigid " << side;
  IMP::benchmark::report(oss.str(), name, runtime - inittime, value);
}

Restraint *setup(Model *m, bool rpcpf, RigidBodies &rbs) {
  set_log_level(SILENT);
  set_check_level(NONE);
  Particles atoms;
  for (int i = 0; i < 5; ++i) {
    std::string path = IMP::benchmark::get_data_path("small_protein.pdb");
    ParticlesTemp catoms = IMP::get_particles(m,
        IMP::internal::create_particles_from_pdb(path, m));
    IMP_INTERNAL_CHECK(catoms.size() != 0, "What happened to the atoms?");
    atoms.insert(atoms.end(), catoms.begin(), catoms.end());
    IMP_NEW(Particle, rbp, (m));
    RigidBody rbd = RigidBody::setup_particle(rbp, catoms);
    rbs.push_back(rbd);
  }
  for (unsigned int i = 0; i < atoms.size(); ++i) {
    XYZR::setup_particle(atoms[i], 1.0);
  }
  IMP_NEW(IMP::internal::StaticListContainer<
              IMP::SingletonContainer>,
          lsc, (m, "list"));

  PairContainer *cpc;
  if (rpcpf) {
    ParticleIndexes rbsp(rbs.size());
    for (unsigned int i = 0; i < rbs.size(); ++i) {
      rbsp[i] = rbs[i].get_particle()->get_index();
    }
    lsc->set(rbsp);
    IMP_NEW(RigidClosePairsFinder, rcps, ());
    cpc = new container::internal::ClosePairContainer(lsc, 0.0, rcps);
  } else {
    IMP_NEW(GridClosePairsFinder, cpf, ());
    lsc->set(IMP::get_indexes(get_as<ParticlesTemp>(atoms)));
    cpc = new container::internal::ClosePairContainer(lsc, 0.0, cpf, 1.0);
  }
  return IMP::create_restraint(new DistancePairScore(new Linear(1, 0)), cpc);
}
}

int main(int argc, char **argv) {
  IMP::setup_from_argv(argc, argv, "Benchmark rigid collisions");
  {
    RigidBodies rbs;
    IMP::PointerMember<Model> m(new IMP::Model());
    IMP::PointerMember<Restraint> r = setup(m, false, rbs);
    // std::cout << "Quadratic:" << std::endl;
    test_one("quadratic", r, rbs, 10, 2.578245);
    test_one("quadratic", r, rbs, 30, 1.919732);
  }
  {
    RigidBodies rbs;
    IMP::PointerMember<Model> m(new IMP::Model());
    IMP::PointerMember<Restraint> r = setup(m, true, rbs);
    // std::cout << "Hierarchy:" << std::endl;
    test_one("hierarchy", r, rbs, 10, 11.549620);
    test_one("hierarchy", r, rbs, 30, 5.830277);
  }
  return IMP::benchmark::get_return_value();
}
