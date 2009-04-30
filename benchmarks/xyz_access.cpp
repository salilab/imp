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

// TEST 1
double compute_distances_decorator_access(
   const IMP::Particles& particles) {
  double tdist=0;
  for (unsigned int i = 0; i < particles.size(); i++) {
    IMP::algebra::Vector3D v1 =
      IMP::core::XYZDecorator(particles[i]).get_coordinates();
    for (unsigned int j = i + 1; j < particles.size(); j++) {
      IMP::algebra::Vector3D v2 =
        IMP::core::XYZDecorator(particles[j]).get_coordinates();
      tdist+= distance(v1, v2);
    }
  }
  return tdist;
}

// TEST 2
class MyParticle : public IMP::Particle {
public:
  MyParticle(IMP::Model *m) : IMP::Particle(m) {}
  IMP::algebra::Vector3D v_;
};


double compute_distances_class_access(
   const std::vector<MyParticle*>& particles) {
  double tdist=0;
  for (unsigned int i = 0; i < particles.size(); i++) {
    for (unsigned int j = i + 1; j < particles.size(); j++) {
      tdist += distance(particles[i]->v_, particles[j]->v_);
    }
  }
  return tdist;
}

// TEST 2.5
class MyParticle2 : public IMP::Particle {
public:
  MyParticle2(IMP::Model *m) : IMP::Particle(m) {
    v_= new IMP::algebra::Vector3D();
  }
  IMP::algebra::Vector3D *v_;
};


double compute_distances_class_access(
    const std::vector<MyParticle2*>& particles) {
  double tdist=0;
  for (unsigned int i = 0; i < particles.size(); i++) {
    for (unsigned int j = i + 1; j < particles.size(); j++) {
      tdist+= distance(*particles[i]->v_, *particles[j]->v_);
    }
  }
  return tdist;
}

// TEST 3
double compute_distances_direct_access(
   const std::vector<Vector3D>& coordinates) {
  double tdist=0;
  for (unsigned int i = 0; i < coordinates.size(); i++) {
    for (unsigned int j = i + 1; j < coordinates.size(); j++) {
      tdist+= distance(coordinates[i], coordinates[j]);
    }
  }
  return tdist;
}

int main(int argc, char **argv) {

  if(argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <pdb file> " << std::endl;
    return 0;
  }
  // read pdb, prepare particles
  IMP::Model *model = new IMP::Model();
  MolecularHierarchyDecorator mhd
    = read_pdb(argv[1], model, NonWaterNonHydrogenSelector());
  std::vector < IMP::Particle * > particles =
      get_by_type(mhd, MolecularHierarchyDecorator::ATOM); ;
  std::cout << "Number of particles " << particles.size() << std::endl;
  set_check_level(IMP::NONE);
  set_log_level(SILENT);
  {
    // TEST 1
    double runtime, dist;
    // measure time
    IMP_TIME(
             {
               dist=compute_distances_decorator_access(particles);
             }, runtime);
    std::cout << "TEST1 (decorator_access)  took " << runtime
              << " (" << dist << ")"<< std::endl;
  }
  {
    // TEST 2
    std::vector < MyParticle * > my_particles;
    for (unsigned int i = 0; i < particles.size(); i++) {
      MyParticle *p = new MyParticle(model);
      p->v_ = IMP::core::XYZDecorator::cast(particles[i]).get_coordinates();
      my_particles.push_back(p);
    }
    double runtime, dist;
    // measure time
    IMP_TIME(
             {
               dist=compute_distances_class_access(my_particles);
             }, runtime);
    std::cout << "TEST2 (class access) took " << runtime
              << " (" << dist << ")"<< std::endl;
  }

  // TEST 2.5
  {
    std::vector < MyParticle2 * > my_particles;
    for (unsigned int i = 0; i < particles.size(); i++) {
      MyParticle2 *p = new MyParticle2(model);
      *p->v_ = IMP::core::XYZDecorator::cast(particles[i]).get_coordinates();
      my_particles.push_back(p);
    }
    double runtime, dist;
    // measure time
    IMP_TIME(
             {
               dist=compute_distances_class_access(my_particles);
             }, runtime);
    std::cout << "TEST2.5 (class access) took " << runtime
              << " (" << dist << ")"<< std::endl;
  }
  // TEST 3
  {
    std::vector<IMP::algebra::Vector3D> coordinates;
    for (unsigned int i = 0; i < particles.size(); i++) {
      coordinates.push_back(IMP::core::XYZDecorator::cast(particles[i]).
                            get_coordinates());
    }
    double runtime, dist;
    // measure time
    IMP_TIME(
             {
               dist=compute_distances_direct_access(coordinates);
             }, runtime);
    std::cout << "TEST3 (direct access) took " << runtime
              << " (" << dist << ")"<< std::endl;
  }
  return 0;
}
