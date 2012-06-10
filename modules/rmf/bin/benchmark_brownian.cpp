/**
 * Copyright 2007-2012 IMP Inventors. All rights reserved.
 */
#include <IMP/PairPredicate.h>
#include <IMP/atom/BrownianDynamics.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/atom/Mass.h>
#include <IMP/atom/Selection.h>
#include <IMP/base/SetLogState.h>
#include <IMP/base/log_macros.h>
#include <IMP/benchmark/benchmark_macros.h>
#include <IMP/benchmark/command_line_macros.h>
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
const double len=r;
const double kk=1000;
const double sigma=.1;
const double slack=6;

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




core::Mover* create_serial_mover(const ParticlesTemp &ps) {
  core::Movers movers;
  for (unsigned int i=0; i< ps.size(); ++i) {
    double scale= core::XYZR(ps[i]).get_radius();
    movers.push_back(new core::BallMover(ParticlesTemp(1, ps[i]),
                                         scale*2));
  }
  IMP_NEW(core::SerialMover, sm, (get_as<core::MoversTemp>(movers)));
  return sm.release();
}


/** Take a set of core::XYZR particles and relax them relative to a set of
    restraints. Excluded volume is handle separately, so don't include it
in the passed list of restraints. */
void optimize_balls(const ParticlesTemp &ps,
                    const RestraintsTemp &rs,
                    const PairPredicates &excluded,
                    const OptimizerStates &opt_states,
                    base::LogLevel ll) {
  // make sure that errors and log messages are marked as coming from this
  // function
  IMP_FUNCTION_LOG;
  base::SetLogState sls(ll);
  IMP_USAGE_CHECK(!ps.empty(), "No Particles passed.");
  Model *m= ps[0]->get_model();
  //double scale = core::XYZR(ps[0]).get_radius();

  IMP_NEW(core::SoftSpherePairScore, ssps, (10));
  IMP_NEW(core::ConjugateGradients, cg, (m));
  cg->set_score_threshold(.1);
  cg->set_optimizer_states(opt_states);
  {
    cg->set_score_threshold(ps.size()*.1);
    // set up restraints for cg
    IMP_NEW(container::ListSingletonContainer, lsc, (ps));
    IMP_NEW(container::ClosePairContainer, cpc,
            (lsc, 0, core::XYZR(ps[0]).get_radius()));
    cpc->add_pair_filters(excluded);
    Pointer<Restraint> r= container::create_restraint(ssps.get(),
                                                      cpc.get());
    r->set_model(ps[0]->get_model());
    cg->set_restraints(rs+RestraintsTemp(1, r.get()));
    cg->set_optimizer_states(opt_states);
  }
  IMP_NEW(core::MonteCarlo, mc, (m));
  mc->set_score_threshold(.1);
  mc->set_optimizer_states(opt_states);
  IMP_NEW(core::IncrementalScoringFunction, isf, (ps, rs));
  {
    // set up MC
    mc->set_score_threshold(ps.size()*.1);
    mc->add_mover(create_serial_mover(ps));
    // we are special casing the nbl term for montecarlo, but using all for CG
    mc->set_incremental_scoring_function(isf);
    // use special incremental support for the non-bonded part
    isf->add_close_pair_score(ssps, 0, ps, excluded);
    // make pointer vector
  }

  IMP_LOG(PROGRESS, "Performing initial optimization" << std::endl);
  {
    boost::ptr_vector<ScopedSetFloatAttribute> attrs;
    for (unsigned int j=0; j< attrs.size(); ++j) {
      attrs.push_back( new ScopedSetFloatAttribute(ps[j],
                                                   core::XYZR::get_radius_key(),
                                                   0));
    }
    cg->optimize(1000);
  }
  // shrink each of the particles, relax the configuration, repeat
  for (int i=0; i< 11; ++i) {
    boost::ptr_vector<ScopedSetFloatAttribute> attrs;
    double factor=.1*i;
    IMP_LOG(PROGRESS, "Optimizing with radii at " << factor << " of full"
            << std::endl);
    for (unsigned int j=0; j< ps.size(); ++j) {
      attrs.push_back( new ScopedSetFloatAttribute(ps[j],
                                                   core::XYZR::get_radius_key(),
                                                   core::XYZR(ps[j])
                                                   .get_radius()*factor));
    }
    // changed all radii
    isf->set_moved_particles(isf->get_movable_particles());
    for (int j=0; j< 5; ++j) {
      mc->set_kt(100.0/(3*j+1));
      mc->optimize(ps.size()*(j+1)*100);
      double e=cg->optimize(10);
      IMP_LOG(PROGRESS, "Energy is " << e << std::endl);
      if (e < .000001) break;
    }
  }
}



It create_particles() {
  It ret;
  ret.m= new Model();
  for (unsigned int i=0; i< 10; ++i) {
    for (unsigned int j=0; j< 10; ++j) {
      atom::Hierarchy parent
          = atom::Hierarchy::setup_particle(new Particle(ret.m));
      std::ostringstream oss;
      oss << "chain " << i << " " << j;
      parent->set_name(oss.str());
      ret.chains.push_back(parent);
      for (unsigned int k=0; k< 10; ++k) {
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
        d.set_coordinates(Vector3D(i*30.0, j*30.0, k*len));
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
  return ret;
}

void write_particles(It cur, RMF::FileHandle rh, int frame) {
  if (frame==0) {
    for (unsigned int i=0; i< cur.chains.size(); ++i) {
      IMP::rmf::add_hierarchy(rh, cur.chains[i]);
    }
    IMP::rmf::add_particle(rh, cur.sp);
  } else {
    IMP::rmf::save_frame(rh, frame);
  }
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
    ret.rss.push_back(container::create_restraint(link, cpc.get()));
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
  ret.bd->set_scoring_function(all_restraints);
  //double ts=Diffusion(ret.all[0]).get_time_step_from_sigma(sigma);
  ret.bd->set_maximum_time_step(ret.sp->get_value(tsk));
  //std::cout << ret.sp->get_value(tsk) << std::endl;
  ret.bd->set_maximum_move(10);
  // from test below is 640
  return ret;
}


double simulate(It it, int ns, bool verbose=false) {
  if (!verbose) it.bd->set_log_level(SILENT);
  return it.bd->optimize(ns);
}

void initialize(It it) {
  //double e=simulate(it, 1e6,true);
  //std::cout << "Time step is " << it.sp.get_maximum_time_step()
  // << std::endl;
  it.m->set_gather_statistics(true);
  std::cout << "Relaxing from " << it.m->evaluate(false) << std::endl;
  //it.m->show_restraint_score_statistics();
  //it.m->set_gather_statistics(false);
  optimize_balls(atom::Selection(it.chains).get_selected_particles(),
                 it.rss, PairPredicates(1, it.filt), OptimizerStates(),
                 IMP::SILENT);
  double slack;
  {
    std::cout << "Estimating slack " << std::endl;
    SetLogState sl(VERBOSE);
    slack= get_slack_estimate(it.lsc->get_particles(), 100, 1,
            get_restraints(RestraintsTemp(1, it.m->get_root_restraint_set())),
                              true,
                              it.bd,
                              it.cpc);
  }
  it.sp->set_value(FloatKey("slack"), slack);
  simulate(it, 500);
  std::cout << "Energy is " << it.m->evaluate(false) << std::endl;
}


void rigidify(const ParticlesTemp &ps, bool no_members) {
  Model *m=ps[0]->get_model();
  for (unsigned int i=0; i< ps.size(); ++i) {
    XYZR d(ps[i]);
    ReferenceFrame3D rf(Transformation3D(Rotation3D(),
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
void do_benchmark(std::string name, int argc, char *argv[], PS0 *link,
                  PS1 *lb, SS *bottom, bool rigid=false,
                  bool no_members=false) ATTRIBUTES;

template <int I, class PR, class PS0, class PS1, class SS>
void do_benchmark(std::string name, int argc, char *argv[], PS0 *link,
                  PS1 *lb, SS *bottom, bool rigid,
                  bool no_members) {
  Pointer<PS0> rclink(link);
  Pointer<PS1> rclb(lb);
  Pointer<SS> rcbottom(bottom);
  std::string in;
  if (argc >0) {
    in =argv[0];
  } else {
    IMP_CATCH_AND_TERMINATE(in
                            =IMP::benchmark::get_data_path("brownian.rmf"));
  }
  It o= create_particles(in);
  if (rigid) {
    for (unsigned int i=0; i< o.chains.size(); ++i) {
      rigidify(get_as<ParticlesTemp>(get_leaves(o.chains[i])),
               no_members);
    }
  }
  It it= create_restraints<PR>(link, lb, bottom, o);
  double total=0, runtime=0;
  int ns=1000;
  if (argc >1) {
    ns=atoi(argv[1]);
  }
  IMP_TIME(
      {
        total+=simulate(it, ns);
      }, runtime);
  IMP::benchmark::report(std::string("bd ")+name, runtime, total);
  if (argc>2) {
    IMP_CATCH_AND_TERMINATE(write_particles(it,
                                            RMF::create_rmf_file(argv[2]),
                                            0));
  }
}
}
//new LowerBound(kk)

int main(int argc , char **argv) {
  bool create_option=false;
  bool initialize_option=false;
  IMP_BENCHMARK(("initialize",
                 boost::program_options::value<bool>
                 (&initialize_option)->zero_tokens(),
                 "Initialize time step and slack.")
                ("setup",
                 boost::program_options::value<bool>
                 (&create_option)->zero_tokens(),
                 "Create a file brownian.rmf."));
  try {
    // shorten lines, ick
    typedef HarmonicLowerBound HLB;
    FloatKey xk=  XYZ::get_xyz_keys()[0];
    if (create_option) {
      It o= create_particles();
      It it=
  create_restraints<PairsRestraint>(new DistancePairScore(new Harmonic(len,kk)),
                                    new SphereDistancePairScore(new HLB(0,kk)),
                                    new AttributeSingletonScore(new HLB(0,kk),
                                                                xk), o);
      {
        RMF::FileHandle fh= RMF::create_rmf_file("brownian.rmf");
        write_particles(it, fh, 0);
      }
      std::cout << it.m->evaluate(false) << " is the score " << std::endl;
      initialize(it);
      RMF::FileHandle fh= RMF::create_rmf_file(argv[2]);
      write_particles(it, fh, 0);
    } else if (initialize_option) {
      It cur;
      cur= create_particles(IMP::benchmark::get_data_path("brownian.rmf"));
      It it=
  create_restraints<PairsRestraint>(new DistancePairScore(new Harmonic(len,kk)),
                                    new SphereDistancePairScore(new HLB(0,kk)),
                                    new AttributeSingletonScore(new HLB(0,kk),
                                                                xk),
                                            cur);
      double ts= get_maximum_time_step_estimate(it.bd);
      std::cout << "Maximum time step is " << ts;
      //it.sp.set_maximum_time_step(ts);
    } else {
      {
        typedef IMP::internal::ContainerRestraint<SoftSpherePairScore,
            ClosePairContainer> PR;
        do_benchmark<1, PR >("custom rigid", argc-1, argv+1,
                             new HarmonicDistancePairScore(len, kk),
                             new SoftSpherePairScore(kk),
                             new AttributeSingletonScore(hlb,
                                                    XYZ::get_xyz_keys()[0]),
                             true);
        do_benchmark<1, PR >("custom rigid no members", argc-1, argv+1,
                             new HarmonicDistancePairScore(len, kk),
                             new SoftSpherePairScore(kk),
                             new AttributeSingletonScore(hlb,
                                                      XYZ::get_xyz_keys()[0]),
                             true, true);
        do_benchmark<1, PR >("custom", argc-1, argv+1,
                             new HarmonicDistancePairScore(len, kk),
                             new SoftSpherePairScore(kk),
                          new AttributeSingletonScore(new HLB(0,kk),
                                                      XYZ::get_xyz_keys()[0]));
        do_benchmark<1, PairsRestraint>("scores", argc-1, argv+1,
                                        new HarmonicDistancePairScore(len, kk),
                                        new SoftSpherePairScore(kk),
                                    new AttributeSingletonScore(new HLB(0,kk),
                                                                    xk));
        do_benchmark<0, PairsRestraint>("generic", argc-1, argv+1,
                                   new DistancePairScore(new Harmonic(len,kk)),
                                   new SphereDistancePairScore(new HLB(0,kk)),
                                   new AttributeSingletonScore(new HLB(0,kk),
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
