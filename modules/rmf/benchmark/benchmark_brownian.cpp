/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */
#include <IMP/PairPredicate.h>
#include <IMP/atom/BrownianDynamics.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/atom/Mass.h>
#include <IMP/algebra/vector_generators.h>
#include <IMP/atom/Selection.h>
#include <IMP/base/SetLogState.h>
#include <IMP/base/log_macros.h>
#include <IMP/benchmark/benchmark_macros.h>
#include <IMP/base/flags.h>
#include <IMP/benchmark/utility.h>
#include <IMP/container/ClosePairContainer.h>
#include <IMP/container/ConsecutivePairContainer.h>
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/container/PairsRestraint.h>
#include <IMP/container/SingletonsRestraint.h>
#include <IMP/container/generic.h>
#include <IMP/core/AttributeSingletonScore.h>
#include <IMP/core/BallMover.h>
#include <IMP/core/ConjugateGradients.h>
#include <IMP/core/DistancePairScore.h>
#include <IMP/core/HarmonicLowerBound.h>
#include <IMP/core/MonteCarlo.h>
#include <IMP/core/SerialMover.h>
#include <IMP/core/SphereDistancePairScore.h>
#include <IMP/core/rigid_bodies.h>
#include <IMP/rmf/atom_io.h>
#include <IMP/rmf/restraint_io.h>
#include <IMP/rmf/frames.h>
#include <IMP/rmf/particle_io.h>
#include <IMP/scoped.h>
#include <boost/ptr_container/ptr_vector.hpp>

using namespace IMP;
using namespace IMP::core;
using namespace IMP::algebra;
using namespace IMP::atom;
using namespace IMP::base;
using namespace IMP::container;
using namespace IMP::benchmark;

const double r=10;
const double len=1.5*r;
const double kk=10;
const double sigma=.1;
const double slack=6;
const double pertub_amount=0.0*r;
#if IMP_BUILD>=IMP_RELEASE
const unsigned int num_x=10;
const unsigned int num_y=num_x;
const unsigned int num_per_chain=10;
const unsigned int number_of_steps=1000;
#else
const unsigned int num_x=2;
const unsigned int num_y=num_x;
const unsigned int num_per_chain=5;
const unsinged int number_of_steps=50;
#endif

#ifdef __GNUC__
#define ATTRIBUTES __attribute ((__noinline__))
#else
#define ATTRIBUTES
#endif

namespace {
#ifndef __clang__
void dummy_f_destructor(){}
#endif
}


struct It {
  Pointer<Model> m;
  atom::Hierarchies chains;
  Pointer<Particle> sp;

  Pointer<ListSingletonContainer> lsc;
  Pointer<ClosePairContainer> cpc;
  Restraints rss;
  Pointer<PairScore> lb;
  Pointer<container::ExclusiveConsecutivePairFilter> filt;
  Pointer<BrownianDynamics> bd;
};

namespace {

FloatKey tsk("time step");
FloatKey sk("slack");




It create_particles() {
  It ret;
  ret.m= new Model();
  ret.m->set_log_level(SILENT);
  Sphere3D perturb(Vector3D(0,0,0), pertub_amount);
  for (unsigned int i=0; i< num_x; ++i) {
    for (unsigned int j=0; j< num_y; ++j) {
      atom::Hierarchy parent
          = atom::Hierarchy::setup_particle(new Particle(ret.m));
      std::ostringstream oss;
      oss << "chain " << i << " " << j;
      parent->set_name(oss.str());
      ret.chains.push_back(parent);
      for (unsigned int k=0; k< num_per_chain; ++k) {
        IMP_NEW(Particle, p, (ret.m));
        std::ostringstream oss;
        oss << "bead " << k;
        p->set_name(oss.str());
        atom::Hierarchy cur= atom::Hierarchy::setup_particle(p);
        ret.chains.back().add_child(cur);
        XYZR d= XYZR::setup_particle(p);
        d.set_radius(r);
        /*Diffusion dd=*/ Diffusion::setup_particle(p);
        d.set_coordinates_are_optimized(k!=0);
        d.set_coordinates(Vector3D(i*30.0, j*30.0, k*len+r)
                          + get_random_vector_in(perturb));
        atom::Mass::setup_particle(p, 1);
      }
    }
  }
  ret.sp= new Particle(ret.m);
  ret.sp->set_name("parameters");
  ret.sp->add_attribute(sk, 4);
  ret.sp->add_attribute(tsk, 50);
  return ret;
}

It create_particles(std::string name) {
  RMF::FileConstHandle r= RMF::open_rmf_file_read_only(name);
  It ret;
  ret.m= new Model();
  ret.chains= IMP::rmf::create_hierarchies(r, ret.m);
  ParticlesTemp ps= IMP::rmf::create_particles(r, ret.m);
  IMP_USAGE_CHECK(ps.size()==1, "Wrong number: " << ps);
  IMP::rmf::load_frame(r, r.get_number_of_frames()-1);
  if (ps.empty()) {
    IMP_THROW("No particles read", IOException);
  }
  ret.sp=ps[0];
  for (unsigned int i=0; i< ret.chains.size(); ++i) {
    core::XYZ(ret.chains[i].get_child(0)).set_coordinates_are_optimized(false);
  }
  return ret;
}

template <class PR, class PS0, class PS1, class SS>
It create_restraints(PS0 *link, PS1 *lb, SS *bottom, It in) {
  It ret=in;

  PairPredicates pfs;
  ParticlesTemp all;
  for (unsigned int i=0; i< ret.chains.size(); ++i) {
    ParticlesTemp cur=ret.chains[i].get_children();
    all.insert(all.end(), cur.begin(), cur.end());
    IMP_NEW(ExclusiveConsecutivePairContainer, cpc,(cur));
    // since they all use the same key
    ret.rss.push_back(container::create_restraint(link, cpc.get(), "link %1%"));
  }
  ret.filt=new container::ExclusiveConsecutivePairFilter();
  pfs.push_back(ret.filt);
  ret.lsc=new ListSingletonContainer(all);
  IMP_NEW(ClosePairContainer, cpc, (ret.lsc, 0, ret.sp->get_value(sk)));

  cpc->add_pair_filters(pfs);
  ret.cpc=cpc;
  ret.lb=lb;
  IMP_NEW(PR, pr, (lb, cpc, "close pairs"));
  //ret.rss.push_back(pr);
  IMP_NEW(SingletonsRestraint, sr,
          (bottom, ret.lsc));
  ret.rss.push_back(sr);
  Restraints all_restraints=ret.rss;
  all_restraints.push_back(pr);
  ret.bd= new BrownianDynamics(ret.m);
  ret.bd->set_log_level(SILENT);
  ret.bd->set_scoring_function(all_restraints);
  ret.bd->get_scoring_function()->set_log_level(SILENT);
  //double ts=Diffusion(ret.all[0]).get_time_step_from_sigma(sigma);
  ret.bd->set_maximum_time_step(ret.sp->get_value(tsk));
  //std::cout << ret.sp->get_value(tsk) << std::endl;
  return ret;
}


double simulate(It it, int ns, bool verbose=false) {
  if (!verbose) it.bd->set_log_level(SILENT);
  return it.bd->optimize(ns);
}

  void update_slack_estimate(It it) {
    std::cout << "Estimating slack " << std::endl;
    SetLogState sl(VERBOSE);
    Restraints rt=it.bd->get_scoring_function()->create_restraints();
    double slack= get_slack_estimate(it.lsc->get_particles(), 20, 1,
                                     get_restraints(rt),
                                     true,
                                     it.bd,
                                     it.cpc);
    it.sp->set_value(FloatKey("slack"), slack);
 }

void create(It it, RMF::FileHandle fh) {
  {
    // repeat estimates to get away from start conditions
    for (unsigned int i=0; i< 10; ++i) {
      SetLogState sll(VERBOSE);
      double ts= get_maximum_time_step_estimate(it.bd);
      IMP::rmf::save_frame(fh, 1);
      std::cout << "Maximum time step is " << ts << std::endl;
      it.sp->set_value(tsk, ts);
      IMP::rmf::save_frame(fh, 2);
      it.bd->set_maximum_time_step(ts);
    }
  }
  update_slack_estimate(it);
  IMP::rmf::save_frame(fh, 3);
}

void initialize(It it) {
  it.bd->optimize(1000);
  update_slack_estimate(it);
}


void rigidify(const ParticlesTemp &ps, bool no_members) {
  Model *m=ps[0]->get_model();
  for (unsigned int i=0; i< ps.size(); ++i) {
    XYZR d(ps[i]);
    ReferenceFrame3D
      rf(Transformation3D(algebra::get_identity_rotation_3d(),
                          d.get_coordinates()));
    RigidBody rb=RigidBody::setup_particle(ps[i], rf);
    if (!no_members) {
      IMP_NEW(Particle, op, (m));
      XYZR::setup_particle(op, d.get_sphere());
      rb.add_member(op);
    }
    RigidBodyDiffusion::setup_particle(rb);
  }
}


template <int I, class PR, class PS0, class PS1, class SS>
void do_benchmark(std::string name, PS0 *link,
                  PS1 *lb, SS *bottom, bool rigid=false,
                  bool no_members=false) ATTRIBUTES;

template <int I, class PR, class PS0, class PS1, class SS>
void do_benchmark(std::string name, PS0 *link,
                  PS1 *lb, SS *bottom, bool rigid,
                  bool no_members) {
  Pointer<PS0> rclink(link);
  Pointer<PS1> rclb(lb);
  Pointer<SS> rcbottom(bottom);
  std::string in;
  IMP_CATCH_AND_TERMINATE(in
                          =IMP::benchmark::get_data_path("brownian.rmf"));
  It o= create_particles(in);
  if (rigid) {
    for (unsigned int i=0; i< o.chains.size(); ++i) {
      rigidify(get_as<ParticlesTemp>(get_leaves(o.chains[i])),
               no_members);
    }
  }
  It it= create_restraints<PR>(link, lb, bottom, o);
  double total=0, runtime=0;
  IMP_TIME(
      {
        total+=simulate(it, number_of_steps);
      }, runtime);
  IMP::benchmark::report("bd", name, runtime, total);
}
}
//new LowerBound(kk)
namespace {
  bool FLAGS_initialize=false, FLAGS_setup=false;
  IMP::base::AddBoolFlag ifl("initialize", "Initialize things",
                             &FLAGS_initialize);
  IMP::base::AddBoolFlag sfl("setup", "Setup things", &FLAGS_setup);
}

int main(int argc , char **argv) {
  IMP::base::setup_from_argv(argc, argv, "Benchmark Brownian dynamics.");
  IMP_NEW(HarmonicLowerBound, hlb, (0, kk));
  try {
    FloatKey xk=  XYZ::get_xyz_keys()[0];
    if (FLAGS_setup) {
      It o= create_particles();
      It it=
  create_restraints<PairsRestraint>(new DistancePairScore(new Harmonic(len,kk)),
                                    new SphereDistancePairScore(hlb),
                                    new AttributeSingletonScore(hlb,
                                                                xk), o);
      {
        RMF::FileHandle fh= RMF::create_rmf_file("process.rmf");
        IMP::rmf::add_hierarchies(fh, it.chains);
        IMP::rmf::add_restraints(fh, it.rss);
        IMP::rmf::add_particle(fh, it.sp);
        std::cout << it.bd->get_scoring_function()->evaluate(false)
                  << " is the score " << std::endl;
        IMP::rmf::save_frame(fh, 0);
        Restraints all
            = get_restraints(it.bd->get_scoring_function()
                             ->create_restraints());
        for (unsigned int i=0; i< all.size(); ++i) {
          std::cout << Showable(all[i]) << " " << all[i]->get_last_score()
                    << std::endl;
        }
        std::cout << "Close pairs: " << it.cpc->get_particle_pairs()
                  << std::endl;
        create(it, fh);
      }
      {
        RMF::FileHandle fh= RMF::create_rmf_file("brownian_uninit.rmf");
        IMP::rmf::add_hierarchies(fh, it.chains);
        IMP::rmf::add_restraints(fh, it.rss);
        IMP::rmf::add_particle(fh, it.sp);
        IMP::rmf::save_frame(fh, 0);
      }
    } else if (FLAGS_initialize) {
      It cur;
      cur= create_particles("brownian_uninit.rmf");
      It it=
  create_restraints<PairsRestraint>(new DistancePairScore(new Harmonic(len,kk)),
                                    new SphereDistancePairScore(hlb),
                                    new AttributeSingletonScore(hlb,
                                                                xk),
                                            cur);
      initialize(it);
      RMF::FileHandle fh= RMF::create_rmf_file("brownian.rmf");
      IMP::rmf::add_hierarchies(fh, it.chains);
      IMP::rmf::add_restraints(fh, it.rss);
      IMP::rmf::add_particle(fh, it.sp);
      IMP::rmf::save_frame(fh, 0);
    } else {
      {
        typedef IMP::internal::ContainerRestraint<SoftSpherePairScore,
            ClosePairContainer> PR;
        do_benchmark<1, PR >("custom rigid",
                             new HarmonicDistancePairScore(len, kk),
                             new SoftSpherePairScore(kk),
                             new AttributeSingletonScore(hlb,
                                                    XYZ::get_xyz_keys()[0]),
                             true);
        do_benchmark<1, PR >("custom rigid no members",
                             new HarmonicDistancePairScore(len, kk),
                             new SoftSpherePairScore(kk),
                             new AttributeSingletonScore(hlb,
                                                        XYZ::get_xyz_keys()[0]),
                             true, true);
        do_benchmark<1, PR >("custom",
                             new HarmonicDistancePairScore(len, kk),
                             new SoftSpherePairScore(kk),
                          new AttributeSingletonScore(hlb,
                                                      XYZ::get_xyz_keys()[0]));
        do_benchmark<1, PairsRestraint>("scores",
                                        new HarmonicDistancePairScore(len, kk),
                                        new SoftSpherePairScore(kk),
                                    new AttributeSingletonScore(hlb,
                                                                    xk));
        do_benchmark<0, PairsRestraint>("generic",
                                   new DistancePairScore(new Harmonic(len,kk)),
                                   new SphereDistancePairScore(hlb),
                                   new AttributeSingletonScore(hlb,
                                                                    xk));
      }
    }
  } catch (IMP::base::Exception e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  } catch (std::exception e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
  return IMP::benchmark::get_return_value();
}
