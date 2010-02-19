/**
 * Copyright 2007-2010 Sali Lab. All rights reserved.
 */
#include <IMP.h>
#include <IMP/core.h>
#include <IMP/algebra.h>
#include <IMP/atom.h>
#include <boost/timer.hpp>
#include <IMP/benchmark/utility.h>
#include <IMP/benchmark/macros.h>
#include <IMP/benchmark/hidden_keys.h>

using namespace IMP;
using namespace IMP::core;
using namespace IMP::algebra;
using namespace IMP::atom;
using namespace IMP::benchmark;

#define N 1

#ifdef __GNUC__
#define ATTRIBUTES __attribute ((__noinline__))
#else
#define ATTRIBUTES
#endif

namespace {
VersionInfo get_module_version_info() {
  return IMP::benchmark::get_module_version_info();
}
  std::string get_module_name() {
    return "benchmark";
  }

class DistanceScore: public SingletonModifier {
public:
  algebra::Vector3D v_;
  mutable double score_;
  DistanceScore(): score_(0){}
  IMP_SINGLETON_MODIFIER(DistanceScore);
};

void DistanceScore::do_show(std::ostream &out) const {}

void DistanceScore::apply(Particle *p) const {
  XYZ d(p);
  score_+= IMP::algebra::distance(d.get_coordinates(), v_);
}
}

// TEST 1
double compute_distances_decorator_access(
                     const IMP::Particles& particles) ATTRIBUTES;

double compute_distances_decorator_access(
   const IMP::Particles& particles) {
  double tdist=0;
  for (unsigned int i = 0; i < particles.size(); i++) {
    IMP::core::XYZ d1(particles[i]);
    IMP::algebra::Vector3D v1(d1.get_coordinates());
    for (unsigned int j = 0; j < particles.size(); j++) {
      IMP::core::XYZ d2(particles[j]);
      IMP::algebra::Vector3D v2(d2.get_coordinates());
      tdist+= IMP::algebra::distance(v1, v2);
    }
  }
  return tdist;
}


// TEST 1.5
double compute_distances_particle_access(
                     const IMP::Particles& particles) ATTRIBUTES;

double compute_distances_particle_access(
   const IMP::Particles& particles) {
  FloatKey xk= hidden_keys[0];
  FloatKey yk= hidden_keys[1];
  FloatKey zk= hidden_keys[2];
  double tdist=0;
  for (unsigned int i = 0; i < particles.size(); i++) {
    IMP::algebra::Vector3D v1(particles[i]->get_value(xk),
                              particles[i]->get_value(yk),
                              particles[i]->get_value(zk));
    for (unsigned int j = 0; j < particles.size(); j++) {
      IMP::algebra::Vector3D v2(particles[j]->get_value(xk),
                                particles[j]->get_value(yk),
                                particles[j]->get_value(zk));
      tdist+= IMP::algebra::distance(v1, v2);
    }
  }
  return tdist;
}

// TEST 5

ParticlesList DistanceScore::get_interacting_particles(Particle *p) const {
  return ParticlesList();
}

ParticlesTemp DistanceScore::get_input_particles(Particle *p) const {
  return ParticlesTemp();
}

ParticlesTemp DistanceScore::get_output_particles(Particle *p) const {
  return ParticlesTemp();
}

ContainersTemp DistanceScore::get_input_containers(Particle *p) const {
  return ContainersTemp();
}

ContainersTemp DistanceScore::get_output_containers(Particle *p) const {
  return ContainersTemp();
}

#if 0
double compute_distances_decorator_access(
                        PackedSingletonContainer *ps,
                        DistanceScore *ds) ATTRIBUTES;

double compute_distances_decorator_access(
                        PackedSingletonContainer *ps,
                        DistanceScore *ds) {
  ds->score_=0;
  unsigned int n=ps->get_number_of_particles();
  for (unsigned int i = 0; i < n; i++) {
    ds->v_=XYZ(ps->get_particle(i)).get_coordinates();
    ps->apply(ds);
  }
  return ds->score_;
}
#endif

// TEST 2
class MyParticle : public IMP::Particle {
public:
  MyParticle(IMP::Model *m) : IMP::Particle(m) {}
  IMP::algebra::Vector3D v_;
};


double compute_distances_class_access(
               const std::vector<MyParticle*>& particles) ATTRIBUTES;

double compute_distances_class_access(
   const std::vector<MyParticle*>& particles){
  double tdist=0;
  for (unsigned int i = 0; i < particles.size(); i++) {
    for (unsigned int j = 0; j < particles.size(); j++) {
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
                   const std::vector<MyParticle2*>& particles) ATTRIBUTES;

double compute_distances_class_access(
    const std::vector<MyParticle2*>& particles){
  double tdist=0;
  for (unsigned int i = 0; i < particles.size(); i++) {
    for (unsigned int j = 0; j < particles.size(); j++) {
      tdist+= IMP::algebra::distance(*particles[i]->v_, *particles[j]->v_);
    }
  }
  return tdist;
}

// TEST 3
double compute_distances_direct_access(
                   const std::vector<Vector3D>& coordinates) ATTRIBUTES;

double compute_distances_direct_access(
   const std::vector<Vector3D>& coordinates){
  double tdist=0;
  for (unsigned int i = 0; i < coordinates.size(); i++) {
    for (unsigned int j = 0; j < coordinates.size(); j++) {
      tdist+= IMP::algebra::distance(coordinates[i], coordinates[j]);
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
      tdist+= IMP::algebra::distance(coordinates[i].c, coordinates[j].c);
    }
  }
  return tdist;
}


void do_benchmark(std::string descr, std::string fname) {
  // read pdb, prepare particles
  Model *model = new IMP::Model();
  atom::Hierarchy mhd
    = read_pdb(fname, model, NonWaterNonHydrogenPDBSelector());
  IMP::Particles particles =
    get_by_type(mhd, atom::ATOM_TYPE);
  //std::cout << "Number of particles " << particles.size() << std::endl;
  //set_check_level(IMP::NONE);
  set_log_level(SILENT);
  // TEST 1
  {
    double runtime, dist;
    // measure time
    IMP_TIME_N(
             {
               dist=compute_distances_decorator_access(particles);
             }, runtime, N);
    /*std::cout << "TEST1 (decorator_access)  took " << runtime
      << " (" << dist << ")"<< std::endl;*/
    IMP::benchmark::report("xyz decorator "+descr, runtime, dist);
  }
  // TEST 1.5
  {
    double runtime, dist;
    // measure time
    IMP_TIME_N(
             {
               dist=compute_distances_particle_access(particles);
             }, runtime, N);
    /*std::cout << "TEST1 (decorator_access)  took " << runtime
      << " (" << dist << ")"<< std::endl;*/
    IMP::benchmark::report("xyz particle "+descr, runtime, dist);
  }
  if (0) {
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
    /*std::cout << "TEST2 (class access) took " << runtime
      << " (" << dist << ")"<< std::endl;*/
    IMP::benchmark::report("xyz internal "+descr, runtime, dist);
    for (unsigned int i = 0; i < particles.size(); i++) {
      model->remove_particle(my_particles[i]);
    }
  }

  // TEST 2.5
  if (0) {
    std::vector < MyParticle2 * > my_particles;
    for (unsigned int i = 0; i < particles.size(); i++) {
      MyParticle2 *p = new MyParticle2(model);
      *p->v_
        = IMP::core::XYZ::decorate_particle(particles[i]).get_coordinates();
      my_particles.push_back(p);
    }
    double runtime, dist;
    // measure time
    IMP_TIME_N(
             {
               dist=compute_distances_class_access(my_particles);
             }, runtime, N);
    /*std::cout << "TEST2.5 (class access) took " << runtime
      << " (" << dist << ")"<< std::endl;*/
    IMP::benchmark::report("xyz *internal "+descr, runtime, dist);
    for (unsigned int i = 0; i < my_particles.size(); i++) {
      model->remove_particle(my_particles[i]);
    }
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
    IMP_TIME_N(
             {
               dist=compute_distances_direct_access(coordinates);
             }, runtime, N);
    /*std::cout << "TEST3 (direct access) took " << runtime
      << " (" << dist << ")"<< std::endl;*/
    IMP::benchmark::report("xyz vector "+descr, runtime, dist);
  }
  // TEST 4
  {
    std::vector<VectorHolder> coordinates;
    for (unsigned int i = 0; i < particles.size(); i++) {
      coordinates.push_back(VectorHolder());
      coordinates.back().c=IMP::core::XYZ::decorate_particle(particles[i]).
        get_coordinates();
    }
    double runtime, dist;
    // measure time
    IMP_TIME_N(
             {
               dist=compute_distances_direct_access_space(coordinates);
             }, runtime, N);
    /*std::cout << "TEST3 (direct access) took " << runtime
      << " (" << dist << ")"<< std::endl;*/
    IMP::benchmark::report("xyz vector space "+descr, runtime, dist);
  }
#if 0
  // TEST 5
  {
    double runtime, dist;
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
    /*std::cout << "TEST1 (decorator_access)  took " << runtime
      << " (" << dist << ")"<< std::endl;*/
    IMP::benchmark::report("xyz decorator packed "+descr, runtime, dist);
    for (unsigned int i=0; i< particles.size(); ++i) {
      model->remove_particle(psc->get_particle(i));
    }
  }
#endif
}

int main(int argc, char **argv) {
  if (argc >1) {
    switch (argv[1][0]) {
    case 's':
      do_benchmark("small",
                   IMP::benchmark::get_data_path("small_protein.pdb"));
      break;
    case 'l':
      do_benchmark("large",
                   IMP::benchmark::get_data_path("large_protein.pdb"));
      break;
    case 'h':
      do_benchmark("huge",
                   IMP::benchmark::get_data_path("huge_protein.pdb"));
      break;
    }
  } else {
    do_benchmark("small",
                 IMP::benchmark::get_data_path("small_protein.pdb"));
    do_benchmark("large",
                 IMP::benchmark::get_data_path("large_protein.pdb"));
    do_benchmark("huge",
                 IMP::benchmark::get_data_path("huge_protein.pdb"));
  }
  return 0;
}
