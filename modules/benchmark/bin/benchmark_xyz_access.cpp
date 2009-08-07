/**
 * Copyright 2007-9 Sali Lab. All rights reserved.
 */
#include <IMP.h>
#include <IMP/core.h>
#include <IMP/algebra.h>
#include <IMP/atom.h>
#include <boost/timer.hpp>
#include <IMP/benchmark/utility.h>

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
      IMP::core::XYZ(particles[i]).get_coordinates();
    for (unsigned int j = i + 1; j < particles.size(); j++) {
      IMP::algebra::Vector3D v2 =
        IMP::core::XYZ(particles[j]).get_coordinates();
      tdist+= IMP::algebra::distance(v1, v2);
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
      tdist += IMP::algebra::distance(particles[i]->v_, particles[j]->v_);
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
      tdist+= IMP::algebra::distance(*particles[i]->v_, *particles[j]->v_);
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
      tdist+= IMP::algebra::distance(coordinates[i], coordinates[j]);
    }
  }
  return tdist;
}

void do_benchmark(std::string descr, std::string fname) {
  // read pdb, prepare particles
  Model *model = new IMP::Model();
  atom::Hierarchy mhd
    = read_pdb(fname, model, NonWaterNonHydrogenSelector());
  IMP::Particles particles =
    get_by_type(mhd, atom::Hierarchy::ATOM);
  //std::cout << "Number of particles " << particles.size() << std::endl;
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
    /*std::cout << "TEST1 (decorator_access)  took " << runtime
      << " (" << dist << ")"<< std::endl;*/
    IMP::benchmark::report("xyz vector "+descr, runtime, dist);
  }
  {
    // TEST 2
    std::vector < MyParticle * > my_particles;
    for (unsigned int i = 0; i < particles.size(); i++) {
      MyParticle *p = new MyParticle(model);
      p->v_ = IMP::core::XYZ::decorate_particle(particles[i]).get_coordinates();
      my_particles.push_back(p);
    }
    double runtime, dist;
    // measure time
    IMP_TIME(
             {
               dist=compute_distances_class_access(my_particles);
             }, runtime);
    /*std::cout << "TEST2 (class access) took " << runtime
      << " (" << dist << ")"<< std::endl;*/
    IMP::benchmark::report("xyz internal "+descr, runtime, dist);
  }

  // TEST 2.5
  {
    std::vector < MyParticle2 * > my_particles;
    for (unsigned int i = 0; i < particles.size(); i++) {
      MyParticle2 *p = new MyParticle2(model);
      *p->v_
        = IMP::core::XYZ::decorate_particle(particles[i]).get_coordinates();
      my_particles.push_back(p);
    }
    double runtime, dist;
    // measure time
    IMP_TIME(
             {
               dist=compute_distances_class_access(my_particles);
             }, runtime);
    /*std::cout << "TEST2.5 (class access) took " << runtime
      << " (" << dist << ")"<< std::endl;*/
    IMP::benchmark::report("xyz *internal "+descr, runtime, dist);
  }
  // TEST 3
  {
    std::vector<IMP::algebra::Vector3D> coordinates;
    for (unsigned int i = 0; i < particles.size(); i++) {
      coordinates.push_back(IMP::core::XYZ::decorate_particle(particles[i]).
                            get_coordinates());
    }
    double runtime, dist;
    // measure time
    IMP_TIME(
             {
               dist=compute_distances_direct_access(coordinates);
             }, runtime);
    /*std::cout << "TEST3 (direct access) took " << runtime
      << " (" << dist << ")"<< std::endl;*/
    IMP::benchmark::report("xyz decorator "+descr, runtime, dist);
  }
}

int main(int argc, char **argv) {
  do_benchmark("small", IMP::get_data_directory()
               +"/benchmark/single_protein.pdb");
  do_benchmark("large", IMP::get_data_directory()
               +"/benchmark/big_protein.pdb");
  return 0;
}
