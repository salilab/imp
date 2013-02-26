/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */
#include <IMP.h>
#include <IMP/core.h>
#include <IMP/algebra.h>
#include <boost/timer.hpp>
#include <IMP/benchmark/utility.h>
#include <IMP/benchmark/benchmark_macros.h>
#include <IMP/base/flags.h>

using namespace IMP;
using namespace IMP::core;
using namespace IMP::algebra;
using namespace IMP::benchmark;


#ifdef __GNUC__
#define ATTRIBUTES __attribute ((__noinline__))
#else
#define ATTRIBUTES
#endif

IMP_COMPILER_ENABLE_WARNINGS

namespace {

// TEST 1
double compute_distances_decorator_access(
                     const IMP::ParticlesTemp& particles) ATTRIBUTES;

double compute_distances_decorator_access(
   const IMP::ParticlesTemp& particles) {
  double tdist=0;
  for (unsigned int i = 0; i < particles.size(); i++) {
    IMP::core::XYZ d1(particles[i]);
    IMP::algebra::Vector3D v1(d1.get_coordinates());
    for (unsigned int j = 0; j < particles.size(); j++) {
      IMP::core::XYZ d2(particles[j]);
      IMP::algebra::Vector3D v2(d2.get_coordinates());
      tdist+= IMP::algebra::get_distance(v1, v2);
    }
  }
  return tdist;
}


// TEST 1.1
double compute_distances_particle_access(
                     const IMP::ParticlesTemp& particles) ATTRIBUTES;

double compute_distances_particle_access(
   const IMP::ParticlesTemp& particles) {
  FloatKey xk("x");
  FloatKey yk("y");
  FloatKey zk("z");
  double tdist=0;
  for (unsigned int i = 0; i < particles.size(); i++) {
    IMP::algebra::Vector3D v1(particles[i]->get_value(xk),
                              particles[i]->get_value(yk),
                              particles[i]->get_value(zk));
    for (unsigned int j = 0; j < particles.size(); j++) {
      IMP::algebra::Vector3D v2(particles[j]->get_value(xk),
                                particles[j]->get_value(yk),
                                particles[j]->get_value(zk));
      tdist+= IMP::algebra::get_distance(v1, v2);
    }
  }
  return tdist;
}

// TEST 2
double compute_distances_no_particle_access(Model *m,
   const IMP::ParticleIndexes& particles) ATTRIBUTES;

  double compute_distances_no_particle_access(Model *m,
   const IMP::ParticleIndexes& particles) {
  double tdist=0;
  for (unsigned int i = 0; i < particles.size(); i++) {
    IMP::core::XYZ d1(m, particles[i]);
    IMP::algebra::Vector3D v1(d1.get_coordinates());
    for (unsigned int j = 0; j < particles.size(); j++) {
      IMP::core::XYZ d2(m, particles[j]);
      IMP::algebra::Vector3D v2(d2.get_coordinates());
      tdist+= IMP::algebra::get_distance(v1, v2);
    }
  }
  return tdist;
}

// TEST 3
double compute_distances_direct_access(
                   const Vector3Ds& coordinates) ATTRIBUTES;

double compute_distances_direct_access(
   const Vector3Ds& coordinates){
  double tdist=0;
  for (unsigned int i = 0; i < coordinates.size(); i++) {
    for (unsigned int j = 0; j < coordinates.size(); j++) {
      tdist+= IMP::algebra::get_distance(coordinates[i], coordinates[j]);
    }
  }
  return tdist;
}


// TEST 3.5
struct VV{
  double x, y, z;
};
double compute_distances_direct_access(
                   const std::vector<VV >& coordinates) ATTRIBUTES;

double compute_distances_direct_access(
   const std::vector<VV >& coordinates){
  double tdist=0;
  for (unsigned int i = 0; i < coordinates.size(); i++) {
    for (unsigned int j = 0; j < coordinates.size(); j++) {
      tdist+= std::sqrt(IMP::square(coordinates[i].x- coordinates[j].x)
                        +IMP::square(coordinates[i].y- coordinates[j].y)
                        +IMP::square(coordinates[i].z- coordinates[j].z));
    }
  }
  return tdist;
}



// TEST 4
struct VectorHolder {
  Vector3D c;
  int a_,b_, c_;
  void *v_;
};

double compute_distances_direct_access_space(
                      const std::vector<VectorHolder>& coordinates)
  ATTRIBUTES;

double compute_distances_direct_access_space(
                      const std::vector<VectorHolder>& coordinates) {
  double tdist=0;
  for (unsigned int i = 0; i < coordinates.size(); i++) {
    for (unsigned int j = 0; j < coordinates.size(); j++) {
      tdist+= IMP::algebra::get_distance(coordinates[i].c, coordinates[j].c);
    }
  }
  return tdist;
}


void do_benchmark(std::string descr, unsigned int n) {
  // read pdb, prepare particles
  Model *model = new IMP::Model();
  ParticlesTemp particles;
  algebra::BoundingBox3D bb= algebra::get_cube_d<3>(100);
  for (unsigned int i=0; i< n; ++i) {
    particles.push_back(new Particle(model));
    core::XYZ::setup_particle(particles.back(),
                              algebra::get_random_vector_in(bb));
  }
  //std::cout << "Number of particles " << particles.size() << std::endl;
  //set_check_level(IMP::NONE);
  set_log_level(SILENT);
  // TEST 1
  {
    double runtime, dist=0;
    // measure time
    IMP_TIME(
             {
               dist+=compute_distances_decorator_access(particles);
               dist+=compute_distances_decorator_access(particles);
             }, runtime);
    /*std::cout << "TEST1 (decorator_access)  took " << runtime
      << " (" << dist << ")"<< std::endl;*/
    IMP::benchmark::report("xyz "+descr, "decorator", runtime, dist);
  }
  // TEST 1.1
  {
    double runtime, dist=0;
    // measure time
    IMP_TIME(
             {
               dist+=compute_distances_particle_access(particles);
               dist+=compute_distances_particle_access(particles);
             }, runtime);
    /*std::cout << "TEST1 (decorator_access)  took " << runtime
      << " (" << dist << ")"<< std::endl;*/
    IMP::benchmark::report("xyz "+descr, "particle", runtime, dist);
  }
  // TEST 1.2
  {
    double runtime, dist=0;
    // measure time
    ParticleIndexes pis(particles.size());
    for (unsigned int i=0; i< pis.size(); ++i) {
      pis[i]= particles[i]->get_index();
    }
    IMP_TIME(
             {
               dist+=compute_distances_no_particle_access(model, pis);
               dist+=compute_distances_no_particle_access(model, pis);
             }, runtime);
    /*std::cout << "TEST1 (decorator_access)  took " << runtime
      << " (" << dist << ")"<< std::endl;*/
    IMP::benchmark::report("xyz "+descr, "decorator from index", runtime, dist);
  }
  /*if (0) {
    // TEST 2
    std::vector < MyParticle * > my_particles;
    for (unsigned int i = 0; i < particles.size(); i++) {
      MyParticle *p = new MyParticle(model);
      p->v_ = IMP::core::XYZ::decorate_particle(particles[i]).get_coordinates();
      my_particles.push_back(p);
    }
    double runtime, dist;
    // measure time
    IMP_TIME_N(
             {
               dist=compute_distances_class_access(my_particles);
             }, runtime, N);
    IMP::benchmark::report("xyz internal "+descr, runtime, targets[2], dist);
    for (unsigned int i = 0; i < particles.size(); i++) {
      model->remove_particle(my_particles[i]);
    }
  }*/

  // TEST 2.5
  /*
  if (0) {
    std::vector < MyParticle2 * > my_particles;
    for (unsigned int i = 0; i < particles.size(); i++) {
      MyParticle2 *p = new MyParticle2(model);
      *p->v_
        = IMP::core::XYZ::decorate_particle(particles[i]).get_coordinates();
      my_particles.push_back(p);
    }
    double runtime, dist=0;
    // measure time
    IMP_TIME_N(
             {
               dist=compute_distances_class_access(my_particles);
             }, runtime, N);
    // out
    IMP::benchmark::report("xyz *internal "+descr, runtime,
    targets[10000], dist);
    for (unsigned int i = 0; i < my_particles.size(); i++) {
      model->remove_particle(my_particles[i]);
    }
    }*/
  // TEST 3
  {
    IMP::algebra::Vector3Ds coordinates;
    for (unsigned int i = 0; i < particles.size(); i++) {
      coordinates.push_back(IMP::core::XYZ::decorate_particle(particles[i]).
                            get_coordinates());
    }
    double runtime, dist=0;
    // measure time
    IMP_TIME(
             {
               dist+=compute_distances_direct_access(coordinates);
               dist+=compute_distances_direct_access(coordinates);
             }, runtime);
    /*std::cout << "TEST3 (direct access) took " << runtime
      << " (" << dist << ")"<< std::endl;*/
    IMP::benchmark::report("xyz "+descr, "vector", runtime, dist);
  }
  {
    IMP::base::Vector<VV > coordinates;
    for (unsigned int i = 0; i < particles.size(); i++) {
      coordinates.push_back(VV());
      coordinates.back().x=IMP::core::XYZ(particles[i]).get_x();
      coordinates.back().y=IMP::core::XYZ(particles[i]).get_y();
      coordinates.back().z=IMP::core::XYZ(particles[i]).get_z();
    }
    double runtime, dist=0;
    // measure time
    IMP_TIME(
             {
               dist+=compute_distances_direct_access(coordinates);
               dist+=compute_distances_direct_access(coordinates);
             }, runtime);
    /*std::cout << "TEST3 (direct access) took " << runtime
      << " (" << dist << ")"<< std::endl;*/
    IMP::benchmark::report("xyz "+descr, "struct", runtime, dist);
  }
  // TEST 4
  {
    IMP::base::Vector<VectorHolder> coordinates;
    for (unsigned int i = 0; i < particles.size(); i++) {
      coordinates.push_back(VectorHolder());
      coordinates.back().c=IMP::core::XYZ::decorate_particle(particles[i]).
        get_coordinates();
    }
    double runtime, dist=0;
    // measure time
    IMP_TIME(
             {
               dist+=compute_distances_direct_access_space(coordinates);
               dist+=compute_distances_direct_access_space(coordinates);
             }, runtime);
    /*std::cout << "TEST3 (direct access) took " << runtime
      << " (" << dist << ")"<< std::endl;*/
    IMP::benchmark::report("xyz "+descr, "vector space", runtime,
                           dist);
  }
  /*
  // TEST 5
  {
    double runtime, dist=0;
    IMP_NEW(PackedSingletonContainer, psc, (model, particles.size()));
    for (unsigned int i=0; i< particles.size(); ++i) {
      XYZ d0(particles[i]);
      XYZ::setup_particle(psc->get_particle(i), d0.get_coordinates());
    }
    IMP_NEW(DistanceScore, ds, ());
    // measure time
    IMP_TIME_N(
             {
               dist=compute_distances_decorator_access(psc, ds);
             }, runtime, N);
    IMP::benchmark::report("xyz decorator packed "+descr, runtime, dist);
    for (unsigned int i=0; i< particles.size(); ++i) {
      model->remove_particle(psc->get_particle(i));
    }
  }
  */
}
}

int main(int argc, char **argv) {
  IMP::base::setup_from_argv(argc, argv,
                             "Benchmark accessing particle attributes");
  do_benchmark("small", 100);
  do_benchmark("large", 1000);
  do_benchmark("huge", 10000);
  return IMP::benchmark::get_return_value();
}
