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

using namespace IMP;
using namespace IMP::core;
using namespace IMP::algebra;
using namespace IMP::atom;

#define N 1

#ifdef __GNUC__
#define ATTRIBUTES __attribute ((__noinline__))
#else
#define ATTRIBUTES
#endif
// TEST 1
double compute_distances_decorator_access(
                     const IMP::Particles& particles) ATTRIBUTES;

double compute_distances_decorator_access(
   const IMP::Particles& particles) {
  double tdist=0;
  for (unsigned int i = 0; i < particles.size(); i++) {
    IMP::algebra::Vector3D v1 =
      IMP::core::XYZ(particles[i]).get_coordinates();
    for (unsigned int j = 0; j < particles.size(); j++) {
      IMP::algebra::Vector3D v2 =
        IMP::core::XYZ(particles[j]).get_coordinates();
      tdist+= IMP::algebra::distance(v1, v2);
    }
  }
  return tdist;
}

// TEST 5

class DistanceScore: public SingletonModifier {
public:
  algebra::Vector3D v_;
  mutable double score_;
  DistanceScore(): score_(0){}
  IMP_SINGLETON_MODIFIER(DistanceScore, VersionInfo());
};

void DistanceScore::show(std::ostream &out) const {}

void DistanceScore::apply(Particle *p) const {
  XYZ d(p);
  score_+= distance(d.get_coordinates(), v_);
}

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
    = read_pdb(fname, model, NonWaterNonHydrogenSelector());
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
